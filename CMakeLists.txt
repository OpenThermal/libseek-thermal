project(libseek-thermal)
cmake_minimum_required(VERSION 2.8)

# Request C++11 standard, using new CMake variables.
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_MODULE_PATH "${libseek-thermal_SOURCE_DIR}/cmake")
include(build_type)

set (WITH_ADDRESS_SANITIZER false CACHE BOOL "Enable address sanitizer")
set (WITH_DEBUG_VERBOSITY false CACHE BOOL "Enable verbose stdout messages")
set (default_build_type "Release")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if (WITH_DEBUG_VERBOSITY)
    add_definitions(-DSEEK_DEBUG)
endif ()

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_package (OpenCV REQUIRED)

add_subdirectory (src)
add_subdirectory (examples)
