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


add_executable(${PROJECT_NAME} main.cpp "CoreDefines.hpp")

create_resource_links(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/static")

add_subdirectory("NoiseGenerator")
add_subdirectory("HomeSite")
add_subdirectory("MCTools")

FILE(GLOB SCP_LIST ${CMAKE_CURRENT_SOURCE_DIR}/NoiseGenerator/views/*.html ${CMAKE_CURRENT_SOURCE_DIR}/HomeSite/views/*.html  ${CMAKE_CURRENT_SOURCE_DIR}/MCTools/views/*.html)
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

target_sources(${PROJECT_NAME} PRIVATE ${VIEWSRC} ${NOISE_SITE_SOURCES} ${HOME_SITE_SOURCES} ${MCTOOLS_SOURCES})

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Drogon::Drogon
        dotenv
        unofficial::noise::noise
        unofficial::noiseutils::noiseutils
)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/NoiseGenerator")
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")



configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.dev.json" "${CMAKE_CURRENT_BINARY_DIR}/config.dev.json" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/.env" "${CMAKE_CURRENT_BINARY_DIR}/.env" COPYONLY)


set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)