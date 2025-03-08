add_subdirectory("assembly")

add_executable(
        z80-test
        main.cpp
        runner.cpp
        test.cpp
        expectation.cpp
        testbattery.cpp
        filereader.cpp
        testfilereader.cpp
        expectationfilereader.cpp
        ../../cpu.cpp
        ../../z80/z80.cpp
        ../../z80/registers.cpp
        ../../z80/iodevice.cpp
        ../../z80/types.cpp
        ../../z80/executionhistory.cpp
        ../../z80/assembly/mnemonic.cpp
        ../../z80/assembly/operand.cpp
        ../../z80/assembly/disassembler.cpp
        testiodevice.h
)

set_target_properties(
        z80-test
        PROPERTIES
        CXX_STANDARD 20
)
