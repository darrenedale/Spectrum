cmake_minimum_required(VERSION 3.0)
project(spectrum LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_BINARY_DIR})

add_executable(
        spectrum
        src/cpu.cpp
        src/imagewidget.cpp
        src/mainwindow.cpp
        src/qspectrumdebugwindow.cpp
        src/qspectrumdisplay.cpp
        src/spectrum.cpp
        src/spectrum_main.cpp
        src/spectrumthread.cpp
        src/z80.cpp
        src/z80iodevice.cpp
)

add_executable(
        z80interpreter
        src/cpu.cpp
        src/imagewidget.cpp
        src/mainwindow.cpp
        src/qspectrumdebugwindow.cpp
        src/qspectrumdisplay.cpp
        src/spectrum.cpp
        src/spectrumthread.cpp
        src/z80.cpp
        src/z80interpreter.cpp
        src/z80interpreter_main.cpp
        src/z80iodevice.cpp
)

set_target_properties(
        spectrum z80interpreter
        PROPERTIES
            CXX_STANDARD 17
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(z80interpreter Qt5::Core Qt5::Gui Qt5::Widgets)
