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
        joystickinterface.cpp
        interfacetwojoystick.cpp
        kempstonjoystick.cpp
        displaydevice.h
        snapshot.cpp
        snapshot.h
        snapshotreader.cpp
        snapshotwriter.cpp
        z80snapshotreader.cpp
        snasnapshotreader.cpp
        zxsnapshotreader.cpp
        z80snapshotwriter.cpp
        z80snapshotwriter.h
        snasnapshotwriter.cpp
        snasnapshotwriter.h
        qt/application.cpp
        qt/mainwindow.cpp
        qt/thread.cpp
        qt/breakpoint.cpp
        qt/programcounterbreakpoint.cpp
        qt/debugwindow.cpp
        qt/pokewidget.cpp
        qt/pokewidget.h
        qt/qimagedisplaydevice.cpp
        qt/registerpairwidget.cpp
        qt/flagswidget.cpp
        qt/keyboard.cpp
        qt/qinterfacetwojoystick.cpp
        qt/qinterfacetwojoystick.h
        qt/qkempstonjoystick.cpp
        qt/memorywidget.cpp
        qt/disassemblywidget.cpp
        qt/threadpauser.h
        qt/keyboardmonitorwidget.cpp
        qt/imagewidget.cpp
        qt/hexspinbox.cpp
        ../cpu.cpp
        ../z80/z80.cpp
        ../z80/iodevice.cpp
        ../z80/registers.cpp
        ../z80/registers.h
        ../z80/assembly/disassembler.cpp
        ../z80/assembly/mnemonic.cpp
        ../z80/assembly/operand.cpp
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Gamepad z)

set_target_properties(
        spectrum
        PROPERTIES
        CXX_STANDARD 20
)

add_subdirectory(screenview)
