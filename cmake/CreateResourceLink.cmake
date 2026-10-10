# cmake/CreateResourceLink.cmake
#
# Script mode (-P). Creates a symbolic link (works on Windows and Unix),
# then touches the stamp file.
#
# On Windows creating a symlink requires either administrator privileges
# or Developer Mode enabled (Windows 10 1703+).
#
# Expected variables:
#   SRC   - source directory (absolute path)
#   DST   - destination link path (absolute path)
#   STAMP - stamp file to touch after success

if(NOT DEFINED SRC OR NOT DEFINED DST OR NOT DEFINED STAMP)
    message(FATAL_ERROR "CreateResourceLink.cmake: SRC, DST and STAMP must be defined")
endif()

if(NOT IS_DIRECTORY "${SRC}")
    message(FATAL_ERROR "Source directory does not exist: ${SRC}")
endif()

if(EXISTS "${DST}")
    message(STATUS "Link already exists: ${DST}")
else()
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${SRC}" "${DST}"
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE out
        ERROR_VARIABLE  err
    )
    if(NOT rc EQUAL 0)
        if(WIN32)
            message(FATAL_ERROR
                "create_symlink failed (rc=${rc}): ${out}${err}\n"
                "On Windows this usually means Developer Mode is disabled and the "
                "process is not running as Administrator. Enable Developer Mode "
                "(Settings -> Update & Security -> For developers) or run the build "
                "from an elevated prompt.")
        else()
            message(FATAL_ERROR "create_symlink failed (rc=${rc}): ${out}${err}")
        endif()
    endif()
    message(STATUS "Created link: ${DST} -> ${SRC}")
endif()

file(TOUCH "${STAMP}")