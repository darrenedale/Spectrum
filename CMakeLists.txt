cmake_minimum_required(VERSION 3.0)
project(spectrum LANGUAGES CXX)

include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(src/spectrum)
add_subdirectory(src/interpreter)

add_executable(
        z80-test
        src/test/z80/main.cpp
        src/test/z80/runner.cpp
        src/test/z80/test.cpp
        src/test/z80/expectation.cpp
        src/test/z80/testbattery.cpp
        src/test/z80/filereader.cpp
        src/test/z80/testfilereader.cpp
        src/test/z80/expectationfilereader.cpp
        src/cpu.cpp
        src/z80/z80.cpp
        src/z80/registers.cpp
        src/z80/iodevice.cpp
        src/test/z80/testiodevice.h)

set_target_properties(
        z80-test
        PROPERTIES
        CXX_STANDARD 20
)
