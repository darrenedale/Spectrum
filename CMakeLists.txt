cmake_minimum_required(VERSION 3.0)
project(spectrum LANGUAGES CXX)

include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(src/spectrum)
add_subdirectory(src/interpreter)
add_subdirectory(src/test)
