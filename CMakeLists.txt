cmake_minimum_required(VERSION 3.0)
project(spectrum LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core Gui Widgets Gamepad)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_BINARY_DIR})

add_executable(
        spectrum
        src/emulator/spectrum.cpp
        src/emulator/application.cpp
        src/emulator/mainwindow.cpp
        src/emulator/main.cpp
        src/cpu.cpp
        src/z80/z80.cpp
        src/z80/z80iodevice.cpp
        src/z80/registers.h
        src/qt/spectrumthread.cpp
        src/qt/ui/qspectrumdebugwindow.cpp
        src/qt/ui/qspectrumdisplay.cpp
        src/qt/ui/imagewidget.cpp
        src/qt/ui/registerpairwidget.cpp src/qt/ui/registerpairwidget.h src/qt/ui/hexspinbox.cpp src/qt/ui/hexspinbox.h src/qt/ui/flagswidget.cpp src/qt/ui/flagswidget.h src/emulator/spectrumkeyboard.cpp src/emulator/spectrumkeyboard.h src/qt/qspectrumkeyboard.cpp src/qt/qspectrumkeyboard.h src/emulator/kempstonjoystick.cpp src/emulator/kempstonjoystick.h src/qt/qkempstonjoystick.cpp src/qt/qkempstonjoystick.h src/qt/ui/memorywidget.cpp src/qt/ui/memorywidget.h src/qt/spectrumthreadpauser.cpp src/qt/spectrumthreadpauser.h)

add_executable(
        screenview
        src/screenview/application.cpp
        src/screenview/mainwindow.cpp
        src/screenview/main.cpp
        src/z80/z80iodevice.cpp
        src/qt/ui/imagewidget.cpp
        src/qt/ui/qspectrumdisplay.cpp
        src/qt/ui/colourcombo.cpp
)

add_executable(
        z80interpreter
        src/cpu.cpp
        src/z80/z80.cpp
        src/interpreter/z80interpreter.cpp
        src/interpreter/main.cpp
)

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
        src/z80/z80iodevice.cpp
)

set_target_properties(
        spectrum z80interpreter screenview z80-test
        PROPERTIES
            CXX_STANDARD 20
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Gamepad)
target_link_libraries(z80interpreter Qt5::Core Qt5::Gui Qt5::Widgets readline)
target_link_libraries(screenview Qt5::Core Qt5::Gui Qt5::Widgets)
