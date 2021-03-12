find_package(Qt5 COMPONENTS Core Gui Widgets Gamepad)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(
        spectrum
        main.cpp
        spectrum.cpp
        z80.cpp
        keyboard.cpp
        kempstonjoystick.cpp
        displaydevice.h
        qt/application.cpp
        qt/mainwindow.cpp
        qt/thread.cpp
        qt/debugwindow.cpp
        qt/qspectrumdisplay.cpp
        qt/registerpairwidget.cpp
        qt/flagswidget.cpp
        qt/keyboard.cpp
        qt/qkempstonjoystick.cpp
        qt/memorywidget.cpp
        qt/threadpauser.h
        qt/keyboardmonitorwidget.cpp
        qt/imagewidget.cpp
        qt/hexspinbox.cpp
        ../cpu.cpp
        ../z80/z80.cpp
        ../z80/iodevice.cpp
        ../z80/registers.h
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Gamepad)

set_target_properties(
        spectrum
        PROPERTIES
        CXX_STANDARD 20
)

add_subdirectory(screenview)
