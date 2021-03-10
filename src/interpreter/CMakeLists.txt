find_package(Qt5 COMPONENTS Core)

add_executable(
        z80interpreter
        ../cpu.cpp
        ../z80/z80.cpp
        z80interpreter.cpp
        main.cpp
)

target_link_libraries(z80interpreter Qt5::Core readline)

set_target_properties(
        z80interpreter
        PROPERTIES
        CXX_STANDARD 20
)
