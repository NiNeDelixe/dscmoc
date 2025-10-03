project(backend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
find_package(Drogon CONFIG REQUIRED)
find_package(unofficial-noise CONFIG REQUIRED)
find_package(unofficial-noiseutils CONFIG REQUIRED)

find_program(DROGON_CTL_EXECUTABLE
    NAMES drogon_ctl
    PATHS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/drogon"
    NO_DEFAULT_PATH
)
message(STATUS "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/drogon")

FILE(GLOB SCP_LIST ${CMAKE_SOURCE_DIR}/views/*.csp ${CMAKE_SOURCE_DIR}/views/*.html)
foreach(cspFile ${SCP_LIST})
  message(STATUS "cspFile:" ${cspFile})
  get_filename_component(classname ${cspFile} NAME_WE)
  message(STATUS "view classname:" ${classname})
  add_custom_command(
    OUTPUT ${classname}.h ${classname}.cc
    COMMAND ${DROGON_CTL_EXECUTABLE} ARGS create view ${cspFile}
    DEPENDS ${cspFile}
    VERBATIM)
  set(VIEWSRC ${VIEWSRC} ${classname}.cc)
endforeach()

add_subdirectory("noise_generator")
add_subdirectory("home_site")

add_executable(${PROJECT_NAME} main.cpp "CoreDefines.hpp" ${VIEWSRC} ${NOISE_SITE_SOURCES} ${HOME_SITE_SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Drogon::Drogon
        dotenv
        unofficial::noise::noise
        unofficial::noiseutils::noiseutils
)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/noise_generator")
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND cmd.exe /C if not exist "views" (mklink /D /J \"${CMAKE_CURRENT_BINARY_DIR}/views\" \"${CMAKE_SOURCE_DIR}/views\")
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND cmd.exe /C if not exist "static" (mklink /D /J \"${CMAKE_CURRENT_BINARY_DIR}/static\" \"${CMAKE_SOURCE_DIR}/static\")
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ln -sf "${CMAKE_SOURCE_DIR}/views" "views"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ln -sf "${CMAKE_SOURCE_DIR}/viestaticws" "static"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()



configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.dev.json" "${CMAKE_CURRENT_BINARY_DIR}/config.dev.json" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/.env" "${CMAKE_CURRENT_BINARY_DIR}/.env" COPYONLY)


set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)