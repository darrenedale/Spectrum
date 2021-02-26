cmake_minimum_required(VERSION 3.0)
project(spectrum LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_BINARY_DIR})

add_executable(
        spectrum
        src/emulator/main.cpp
        src/emulator/application.cpp
        src/emulator/mainwindow.cpp
        src/qt/spectrumthread.cpp
        src/qt/ui/qspectrumdebugwindow.cpp
        src/qt/ui/qspectrumdisplay.cpp
        src/cpu.cpp
        src/qt/ui/imagewidget.cpp
        src/spectrum.cpp
        src/z80/z80.cpp
        src/z80/z80iodevice.cpp
)

add_executable(
        screenview
        src/screenview/main.cpp
        src/screenview/application.cpp
        src/screenview/mainwindow.cpp
        src/qt/ui/imagewidget.cpp
        src/qt/ui/qspectrumdisplay.cpp
        src/screenview/mainwindow.cpp
        src/qt/ui/colourcombo.cpp
)

add_executable(
        z80interpreter
        src/cpu.cpp
        src/qt/ui/imagewidget.cpp
        src/spectrum.cpp
        src/z80/z80.cpp
        src/interpreter/z80interpreter.cpp
        src/interpreter/main.cpp
        src/z80/z80iodevice.cpp
)

set_target_properties(
        spectrum z80interpreter screenview
        PROPERTIES
            CXX_STANDARD 17
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(z80interpreter Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_libraries(screenview Qt5::Core Qt5::Gui Qt5::Widgets)
