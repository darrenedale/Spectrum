option(WITH_QT_GAMEPAD "Include support for host gamepads using the QtGamepad module" OFF)
option(USE_QT5 "Use QT5 instead of QT6" OFF)

set(
        SPECTRUM_QT_COMPONENTS
        Core
        Gui
        Widgets
        DBus
)

set(
        SPECTRUM_QT_LIBS
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::DBus
)

set(
        SPECTRUM_QT_GAMEPAD_SOURCES
)

if (WITH_QT_GAMEPAD)
    list(APPEND SPECTRUM_QT_COMPONENTS Gamepad)
    list(APPEND SPECTRUM_QT_LIBS Qt::Gamepad)
    list(APPEND SPECTRUM_QT_GAMEPAD_SOURCES qtui/gamecontrollerhandler.cpp)
endif(WITH_QT_GAMEPAD)

if (USE_QT5)
    message(STATUS "Using Qt5")
    add_compile_definitions(-DUSE_QT5)
    find_package(Qt5 COMPONENTS ${SPECTRUM_QT_COMPONENTS})
else (USE_QT5)
    message(STATUS "Using Qt6")
    find_package(Qt6 COMPONENTS ${SPECTRUM_QT_COMPONENTS})
endif(USE_QT5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(
        spectrum

        basespectrum.cpp
        main.cpp
        pokedefinition.cpp
        snapshot.cpp
        spectrum128k.cpp
        spectrum16k.cpp
        spectrum48k.cpp
        spectrumplus2.cpp
        spectrumplus2a.cpp
        spectrumplus3.cpp
        types.cpp
        z80.cpp

        debugger/breakpoint.cpp
        debugger/integermemorywatch.h
        debugger/integermemorywatchbase.h
        debugger/memorybreakpoint.h
        debugger/memorychangedbreakpoint.h
        debugger/memorywatch.h
        debugger/programcounterbreakpoint.cpp
        debugger/registerbreakpoint.h
        debugger/registervaluebreakpoint.cpp
        debugger/stackpointerbelowbreakpoint.cpp
        debugger/stringmemorywatch.cpp

        devices/cursorjoystick.cpp
        devices/displaydevice.h
        devices/fullerjoystick.cpp
        devices/interfacetwojoystick.cpp
        devices/joystickinterface.cpp
        devices/kempstonjoystick.cpp
        devices/kempstonmouse.cpp
        devices/keyboard.cpp
        devices/mouseinterface.cpp
        devices/pagingdevice128k.cpp
        devices/pagingdeviceplus2a.cpp

        io/pokfilereader.cpp
        io/snapshotformatguesser.h
        io/snapshotformatmatcher.h
        io/snapshotreader.cpp
        io/snapshotreaderfactory.h
        io/snapshotwriter.cpp
        io/snapshotwriterclass.h
        io/snapshotwriterfactory.h
        io/snasnapshotreader.cpp
        io/snasnapshotwriter.cpp
        io/spsnapshotreader.cpp
        io/spsnapshotwriter.cpp
        io/z80snapshotreader.cpp
        io/z80snapshotwriter.cpp
        io/zx82snapshotreader.cpp
        io/zx82snapshotwriter.cpp
        io/zxsnapshotreader.cpp
        io/zxsnapshotwriter.cpp

        memory/SimpleSpectrumMemory.cpp
        memory/memory128k.cpp
        memory/memoryplus2.h
        memory/memoryplus2a.cpp
        memory/memoryplus3.h
        memory/pagingmemory.h
        memory/pagingmemoryinterface.h

        qtui/aboutwidget.cpp
        qtui/actionableitemview.h
        qtui/actionbar.cpp
        qtui/application.cpp
        qtui/cheatsview.cpp
        qtui/cheatsviewitem.cpp
        qtui/debugger/breakpointscontextmenu.cpp
        qtui/debugger/breakpointsmodel.cpp
        qtui/debugger/breakpointsview.cpp
        qtui/debugger/debugwindow.cpp
        qtui/debugger/disassemblywidget.cpp
        qtui/debugger/flagswidget.cpp
        qtui/debugger/interruptwidget.cpp
        qtui/debugger/keyboardmonitorwidget.cpp
        qtui/debugger/memorycontextmenu.cpp
        qtui/debugger/memorysearchwidget.cpp
        qtui/debugger/memorywidget.cpp
        qtui/debugger/programpointerswidget.cpp
        qtui/debugger/registerswidget.cpp
        qtui/debugger/shadowregisterswidget.cpp
        qtui/debugger/watchescontextmenu.cpp
        qtui/debugger/watchesmodel.cpp
        qtui/debugger/watchesview.cpp
        qtui/dialogue.cpp
        qtui/help.qrc
        qtui/helpwidget.cpp
        qtui/hexspinbox.cpp
        qtui/hexspinboxdelegate.cpp
        qtui/icons.qrc
        qtui/imagewidget.cpp
        qtui/mainwindow.cpp
        qtui/memoryview.cpp
        qtui/notification.cpp
        qtui/pokewidget.cpp
        qtui/pokewidget.h
        qtui/qimagedisplaydevice.cpp
        qtui/registerpairwidget.cpp
        qtui/spectrumdisplayimagewidget.cpp
        qtui/thread.cpp
        qtui/threadpauser.h
        qtui/widgetupdatesuspender.h

        ../../dist/macos/Spectrum/Spectrum.icns
        ../computer.h
        ../cpu.cpp
        ../memory.h
        ../simplememory.h
        ../util/concepts.h
        ../util/crc32.h
        ../util/debug.h
        ../util/endian.h
        ../util/numeric.h
        ../z80/assembly/disassembler.cpp
        ../z80/assembly/mnemonic.cpp
        ../z80/assembly/operand.cpp
        ../z80/executionhistory.cpp
        ../z80/iodevice.cpp
        ../z80/registers.cpp
        ../z80/registers.h
        ../z80/types.cpp
        ../z80/z80.cpp

        ${SPECTRUM_QT_GAMEPAD_SOURCES}
)

target_link_libraries(spectrum ${SPECTRUM_QT_LIBS})

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
