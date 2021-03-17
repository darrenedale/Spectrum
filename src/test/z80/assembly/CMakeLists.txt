
add_executable(
        z80-disassembler
        disassembler.cpp
        ../../../z80/assembly/disassembler.cpp
        ../../../z80/assembly/mnemonic.cpp
        ../../../z80/assembly/operand.cpp
)

set_target_properties(
        z80-disassembler
        PROPERTIES
        CXX_STANDARD 20
)
