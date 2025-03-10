find_package(Qt5 COMPONENTS Core Gui Widgets Gamepad DBus)

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
        spectrumplus3.cpp
        pagingmemoryinterface.h
        pagingmemory.h
        memory.cpp
        memory128k.cpp
        memoryplus2.h
        memoryplus2a.cpp
        memoryplus3.h
        pagingdevice128k.cpp
        pagingdeviceplus2a.cpp
        z80.cpp
        keyboard.cpp
        joystickinterface.cpp
        kempstonjoystick.cpp
        interfacetwojoystick.cpp
        cursorjoystick.cpp
        mouseinterface.cpp
        fullerjoystick.cpp
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
        io/zx82snapshotwriter.cpp
        io/zxsnapshotwriter.cpp
        io/snapshotformatmatcher.h
        io/snapshotformatguesser.h
        io/snapshotreaderfactory.h
        io/snapshotwriterclass.h
        io/snapshotwriterfactory.h
        io/pokfilereader.cpp
        debugger/breakpoint.cpp
        debugger/programcounterbreakpoint.cpp
        debugger/stackpointerbelowbreakpoint.cpp
        debugger/memorybreakpoint.h
        debugger/memorychangedbreakpoint.h
        debugger/registerbreakpoint.h
        debugger/registervaluebreakpoint.cpp
        debugger/memorywatch.h
        debugger/integermemorywatchbase.h
        debugger/integermemorywatch.h
        debugger/stringmemorywatch.cpp
        qtui/application.cpp
        qtui/mainwindow.cpp
        qtui/aboutwidget.cpp
        qtui/helpwidget.cpp
        qtui/cheatsview.cpp
        qtui/cheatsviewitem.cpp
        qtui/thread.cpp
        qtui/actionableitemview.h
        qtui/debugger/debugwindow.cpp
        qtui/debugger/registerswidget.cpp
        qtui/debugger/shadowregisterswidget.cpp
        qtui/debugger/interruptwidget.cpp
        qtui/debugger/programpointerswidget.cpp
        qtui/memoryview.cpp
        qtui/debugger/memorywidget.cpp
        qtui/debugger/memorysearchwidget.cpp
        qtui/debugger/watchesview.cpp
        qtui/debugger/watchesmodel.cpp
        qtui/debugger/breakpointsview.cpp
        qtui/debugger/breakpointsmodel.cpp
        qtui/debugger/watchescontextmenu.cpp
        qtui/debugger/breakpointscontextmenu.cpp
        qtui/debugger/memorycontextmenu.cpp
        qtui/hexspinboxdelegate.cpp
        qtui/pokewidget.cpp
        qtui/pokewidget.h
        qtui/qimagedisplaydevice.cpp
        qtui/registerpairwidget.cpp
        qtui/debugger/flagswidget.cpp
        qtui/debugger/disassemblywidget.cpp
        qtui/threadpauser.h
        qtui/debugger/keyboardmonitorwidget.cpp
        qtui/imagewidget.cpp
        qtui/actionbar.cpp
        qtui/widgetupdatesuspender.h
        qtui/spectrumdisplayimagewidget.cpp
        qtui/hexspinbox.cpp
        qtui/dialogue.cpp
        qtui/gamecontrollerhandler.cpp
        qtui/notification.cpp
        qtui/icons.qrc
        qtui/help.qrc
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
        ../util/endian.h
        ../util/debug.h
        ../util/numeric.h
        ../util/concepts.h
        ../../dist/macos/Spectrum/Spectrum.icns
)

target_link_libraries(spectrum Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Gamepad Qt5::DBus)

set_target_properties(
        spectrum
        PROPERTIES
        CXX_STANDARD 20
        OUTPUT_NAME Spectrum
)

if (APPLE)
    find_library(COREFOUNDATION_FRAMEWORK NAMES CoreFoundation)

    set(
            RESOURCE_FILES
            ../../dist/macos/Spectrum/Spectrum.icns
            ../../roms/spectrum48.rom
            ../../roms/spectrum128-0.rom
            ../../roms/spectrum128-1.rom
            ../../roms/spectrumplus2-0.rom
            ../../roms/spectrumplus2-1.rom
            ../../roms/spectrumplus2-2.rom
            ../../roms/spectrumplus2-3.rom
            ../../roms/spectrumplus3-0.rom
            ../../roms/spectrumplus3-1.rom
            ../../roms/spectrumplus3-2.rom
            ../../roms/spectrumplus3-3.rom
    )

    set_target_properties(
            spectrum
            PROPERTIES
            MACOSX_BUNDLE ON
            RESOURCE "${RESOURCE_FILES}"
            MACOSX_BUNDLE_BUNDLE_NAME Spectrum
            MACOSX_BUNDLE_BUNDLE_VERSION 0.5
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.5"
            MACOSX_BUNDLE_INFO_STRING "A ZX Spectrum emulator"
            MACOSX_BUNDLE_ICON_FILE Spectrum.icns
    )

    target_link_libraries(spectrum "${COREFOUNDATION_FRAMEWORK}")

endif(APPLE)

add_subdirectory(screenview)
