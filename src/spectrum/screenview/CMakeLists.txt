
add_executable(
        screenview
        main.cpp
        application.cpp
        mainwindow.cpp
        colourcombo.cpp
        ../qt/qspectrumdisplay.cpp
        ../qt/imagewidget.cpp
        ../../z80/iodevice.cpp
)

set_target_properties(
        screenview
        PROPERTIES
        CXX_STANDARD 20
)

target_link_libraries(screenview Qt5::Core Qt5::Gui Qt5::Widgets)
