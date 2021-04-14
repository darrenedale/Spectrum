find_package(Qt5 COMPONENTS Core Gui Widgets Gamepad)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(
        spectrum
        main.cpp
        basespectrum.cpp
        spectrum16k.cpp
        spectrum48k.cpp
        spectrum128k.cpp
        spectrumplus2.cpp
        spectrumplus2a.cpp
        spectrum128kmemory.cpp
        spectrum128kpagingdevice.cpp
        spectrumplus2amemory.cpp
        spectrumplus2apagingdevice.cpp
        z80.cpp
        keyboard.cpp
        joystickinterface.cpp
        interfacetwojoystick.cpp
        kempstonjoystick.cpp
        mouseinterface.cpp
        kempstonmouse.cpp
        displaydevice.h
        snapshot.cpp
        pokedefinition.cpp
        types.cpp
        io/snapshotreader.cpp
        io/snapshotwriter.cpp
        io/z80snapshotreader.cpp
        io/snasnapshotreader.cpp
        io/spsnapshotreader.cpp
        io/zxsnapshotreader.cpp
        io/zx82snapshotreader.cpp
        io/z80snapshotwriter.cpp
        io/snasnapshotwriter.cpp
        io/spsnapshotwriter.cpp
        io/pokfilereader.cpp
        qtui/application.cpp
        qtui/mainwindow.cpp
        qtui/pokeswidget.cpp
        qtui/pokeswidgetitem.cpp
        qtui/thread.cpp
        qtui/breakpoint.cpp
        qtui/programcounterbreakpoint.cpp
        qtui/stackpointerbelowbreakpoint.cpp
        qtui/memorybreakpoint.h
        qtui/memorychangedbreakpoint.h
        qtui/registerbreakpoint.h
        qtui/registervaluebreakpoint.cpp
        qtui/debugwindow.cpp
        qtui/registerswidget.cpp
        qtui/shadowregisterswidget.cpp
        qtui/interruptwidget.cpp
        qtui/programpointerswidget.cpp
        qtui/memoryview.cpp
        qtui/memorydebugwidget.cpp
        qtui/custompokewidget.cpp
        qtui/custompokewidget.h
        qtui/qimagedisplaydevice.cpp
        qtui/registerpairwidget.cpp
        qtui/flagswidget.cpp
        qtui/disassemblywidget.cpp
        qtui/threadpauser.h
        qtui/keyboardmonitorwidget.cpp
        qtui/imagewidget.cpp
        qtui/hexspinbox.cpp
        ../computer.h
        ../cpu.cpp
        ../simplememory.h
        ../memory.h
        ../z80/z80.cpp
        ../z80/executionhistory.cpp
        ../z80/iodevice.cpp
        ../z80/registers.cpp
        ../z80/registers.h
        ../z80/assembly/disassembler.cpp
        ../z80/assembly/mnemonic.cpp
        ../z80/assembly/operand.cpp
        ../z80/types.cpp
        ../util/crc32.h
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Gamepad)

set_target_properties(
        spectrum
        PROPERTIES
        CXX_STANDARD 20
)

add_subdirectory(screenview)
