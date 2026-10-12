# cmake/CreateResourceLinks.cmake
#
# Creates symbolic links for the specified directories in the project's
# binary directory.
#
# Usage:
#   create_resource_links(${PROJECT_NAME} views static)

set(_CRL_SCRIPT_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(create_resource_links _TARGET)
    if(NOT ARGN)
        message(WARNING "create_resource_links: no directories specified for linking")
        return()
    endif()

    if(NOT TARGET ${_TARGET})
        message(FATAL_ERROR "create_resource_links: target '${_TARGET}' does not exist")
    endif()

    string(MD5 dir_hash "${CMAKE_CURRENT_BINARY_DIR}")
    string(SUBSTRING "${dir_hash}" 0 8 dir_hash)

    foreach(resource IN LISTS ARGN)
        get_filename_component(res_name ${resource} NAME)
        set(stamp "${CMAKE_CURRENT_BINARY_DIR}/.link_${_TARGET}_${res_name}_${dir_hash}")
        if(IS_ABSOLUTE ${resource})
            set(src "${resource}")
        else()
            set(src "${CMAKE_CURRENT_SOURCE_DIR}/${resource}")
        endif()
        
        
        set(dst "${CMAKE_CURRENT_BINARY_DIR}/${res_name}")

        add_custom_command(
            OUTPUT  "${stamp}"
            COMMAND ${CMAKE_COMMAND}
                    -DSRC=${src}
                    -DDST=${dst}
                    -DSTAMP=${stamp}
                    -P "${_CRL_SCRIPT_DIR}/CreateResourceLink.cmake"
            COMMENT "Creating link for ${resource}"
            VERBATIM
        )

        set(link_target "${_TARGET}_link_${res_name}_${dir_hash}")
        add_custom_target(${link_target} ALL DEPENDS "${stamp}")
        add_dependencies(${_TARGET} ${link_target})
    endforeach()
endfunction()


# cmake/CreateDrogonViews.cmake
#
# Generates Drogon view classes (.h + .cc) from *.csp / *.html templates
# using the drogon_ctl executable, and appends the generated .cc files
# to a parent-scope variable.
#
# Usage:
#   create_drogon_views(VIEWSRC views)
#   create_drogon_views(VIEWSRC views other/views_dir)
#
# Arguments:
#   OUT_VAR  - name of the variable in the parent scope where the list of
#              generated .cc files is appended
#   ARGN     - one or more directories to scan for *.csp / *.html templates.
#              Each entry may be absolute or relative to CMAKE_SOURCE_DIR.
#
# Requires:
#   DROGON_CTL_EXECUTABLE - path to the drogon_ctl executable
#                           (e.g. found via find_program)

function(create_drogon_views OUT_VAR)
    if(NOT ARGN)
        message(WARNING "create_drogon_views: no source directories specified")
        return()
    endif()

    if(NOT DROGON_CTL_EXECUTABLE)
        message(FATAL_ERROR
            "create_drogon_views: DROGON_CTL_EXECUTABLE is not set. "
            "Use find_program(DROGON_CTL_EXECUTABLE drogon_ctl) before calling.")
    endif()

    set(generated_cc "")
    set(work_dir "${CMAKE_CURRENT_BINARY_DIR}/drogon_views")

    foreach(dir IN LISTS ARGN)
        if(IS_ABSOLUTE "${dir}")
            set(views_dir "${dir}")
        else()
            set(views_dir "${CMAKE_SOURCE_DIR}/${dir}")
        endif()

        file(GLOB_RECURSE view_files CONFIGURE_DEPENDS
            "${views_dir}/*.csp"
            "${views_dir}/*.html"
        )

        if(NOT view_files)
            message(STATUS "create_drogon_views: no templates found in ${views_dir}")
            continue()
        endif()

        foreach(view_file IN LISTS view_files)
            get_filename_component(classname "${view_file}" NAME_WE)
            get_filename_component(ext       "${view_file}" EXT)
            message(STATUS "create_drogon_views: ${classname} <- ${view_file}")

            set(csp_copy "${work_dir}/${classname}${ext}")
            set(header   "${work_dir}/${classname}.h")
            set(source   "${work_dir}/${classname}.cc")

            add_custom_command(
                OUTPUT  "${header}" "${source}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${work_dir}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${view_file}" "${csp_copy}"
                COMMAND ${DROGON_CTL_EXECUTABLE} ARGS create view "${csp_copy}"
                DEPENDS "${view_file}"
                COMMENT "Generating drogon view ${classname}"
                WORKING_DIRECTORY "${work_dir}"
                VERBATIM
            )

            list(APPEND generated_cc "${source}")
        endforeach()
    endforeach()

    set(${OUT_VAR} ${${OUT_VAR}} ${generated_cc} PARENT_SCOPE)
endfunction()