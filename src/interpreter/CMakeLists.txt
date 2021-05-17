add_executable(
        z80interpreter
        ../cpu.cpp
        ../z80/z80.cpp
        ../z80/types.cpp
        ../z80/executionhistory.cpp
        ../z80/registers.cpp
        ../z80/assembly/disassembler.cpp
        ../z80/assembly/mnemonic.cpp
        ../z80/assembly/operand.cpp
        ../util/string.h
        z80interpreter.cpp
        operand.cpp
        main.cpp
)

target_link_libraries(z80interpreter readline)

set_target_properties(
        z80interpreter
        PROPERTIES
        CXX_STANDARD 20
)
