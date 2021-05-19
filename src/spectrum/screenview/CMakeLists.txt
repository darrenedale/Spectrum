
add_executable(
        screenview
        main.cpp
        application.cpp
        mainwindow.cpp
        colourcombo.cpp
        ../qtui/qimagedisplaydevice.cpp
        ../qtui/imagewidget.cpp
        ../../z80/iodevice.cpp
)

set_target_properties(
        screenview
        PROPERTIES
        CXX_STANDARD 20
)

if (APPLE)
    set_target_properties(
            screenview
            PROPERTIES
            MACOSX_BUNDLE ON
            MACOSX_BUNDLE_BUNDLE_NAME "Spectrum ScreenView"
            MACOSX_BUNDLE_BUNDLE_VERSION 0.5
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.5"
            MACOSX_BUNDLE_INFO_STRING "A viewer for ZX Spectrum screen dumps"
    )
endif(APPLE)

target_link_libraries(screenview Qt5::Core Qt5::Gui Qt5::Widgets)
