cmake_minimum_required(VERSION 3.8)
project(raytracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/tools/cmake")

find_package(Catch REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

include(tools/cmake/TestSolution.cmake)
include_directories(tools/util)

add_subdirectory(raytracer-debug)
add_subdirectory(raytracer-geom)
add_subdirectory(raytracer-reader)
add_subdirectory(raytracer)
