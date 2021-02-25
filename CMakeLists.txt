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
        src/application.cpp
        src/application.h
)

add_executable(
        screenview
        src/screenview/main.cpp
        src/screenview/application.cpp
        src/screenview/mainwindow.cpp
        src/imagewidget.cpp
        src/qspectrumdisplay.cpp
        src/screenview/mainwindow.cpp src/screenview/mainwindow.h)

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
        src/interpreter/z80interpreter.cpp
        src/interpreter/main.cpp
        src/z80iodevice.cpp
)

set_target_properties(
        spectrum z80interpreter screenview
        PROPERTIES
            CXX_STANDARD 17
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(z80interpreter Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(screenview Qt5::Core Qt5::Gui Qt5::Widgets)
