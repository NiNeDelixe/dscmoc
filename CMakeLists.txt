cmake_minimum_required(VERSION 3.19)
project(application)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "vcpkg root: ${VCPKG_ROOT}")
message(STATUS "cmake toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

set(BUILD_DOCS OFF)
add_subdirectory("${CMAKE_SOURCE_DIR}/thirdparty/dotenv-cpp")
	
add_subdirectory("backend")