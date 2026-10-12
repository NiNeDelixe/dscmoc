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
if(NOT EXISTS "${DROGON_CTL_EXECUTABLE}")
    log(FATAL "DROGON_CTL_EXECUTABLE points to '${DROGON_CTL_EXECUTABLE}', but the file does not exist")
endif()
log(INFO "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/drogon")


add_executable(${PROJECT_NAME} main.cpp "CoreDefines.hpp")

create_resource_links(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/static")

add_subdirectory("projects/NoiseGenerator")
list(TRANSFORM NOISE_SITE_SOURCES PREPEND "projects/")
add_subdirectory("projects/HomeSite")
list(TRANSFORM HOME_SITE_SOURCES PREPEND "projects/")
add_subdirectory("projects/MCTools")
list(TRANSFORM MCTOOLS_SOURCES PREPEND "projects/")

FILE(GLOB_RECURSE SCP_LIST ${CMAKE_CURRENT_SOURCE_DIR}/projects/NoiseGenerator/views/*.html ${CMAKE_CURRENT_SOURCE_DIR}/projects/HomeSite/views/*.html  ${CMAKE_CURRENT_SOURCE_DIR}/projects/MCTools/views/*.html)
foreach(cspFile ${SCP_LIST})
  log(INFO "cspFile: " ${cspFile})
  get_filename_component(classname ${cspFile} NAME_WE)
  log(INFO "view classname: " ${classname})
  add_custom_command(
    OUTPUT ${classname}.h ${classname}.cc
    COMMAND ${DROGON_CTL_EXECUTABLE} ARGS create view ${cspFile}
    DEPENDS ${cspFile}
    VERBATIM)
  set(VIEWSRC ${VIEWSRC} ${classname}.cc)
  log(INFO "view source:" ${CMAKE_CURRENT_BINARY_DIR}/${classname}.cc)
endforeach()

add_subdirectory(models)

target_sources(${PROJECT_NAME} PRIVATE ${VIEWSRC} "${NOISE_SITE_SOURCES}" "${HOME_SITE_SOURCES}" "${MCTOOLS_SOURCES}")

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
        Drogon::Drogon
        dotenv
        unofficial::noise::noise
        unofficial::noiseutils::noiseutils
)

target_include_directories(
  ${PROJECT_NAME} 
    PRIVATE 
      "${PROJECT_SOURCE_DIR}/projects/NoiseGenerator" 
      "${PROJECT_SOURCE_DIR}"
)



configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json" COPYONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.dev.json" "${CMAKE_CURRENT_BINARY_DIR}/config.dev.json" COPYONLY)
configure_file("${CMAKE_SOURCE_DIR}/.env" "${CMAKE_CURRENT_BINARY_DIR}/.env" COPYONLY)


set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)