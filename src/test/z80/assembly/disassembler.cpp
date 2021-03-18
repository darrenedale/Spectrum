//
// Created by darren on 17/03/2021.
//

#include <fstream>
#include <iostream>
#include <iomanip>

#include "../../../z80/types.h"
#include "../../../z80/assembly/disassembler.h"

using namespace Z80;
using namespace Z80::Assembly;

constexpr const int ErrNoRomFile = 1;
constexpr const int ErrNoRomFileRead = 2;

int main(int argc, char ** argv)
{
    UnsignedByte memory[0x3fff];
    const char * romFileName = "spectrum48.rom";

    if (argc > 1) {
        romFileName = argv[1];
    }

    std::ifstream romFile(romFileName);

    if (!romFile) {
        std::cerr << "failed to open ROM file '" << romFileName << "'\n";
        return ErrNoRomFile;
    }

    romFile.read(reinterpret_cast<std::ifstream::char_type *>(memory), 0x4000);

    if (romFile.bad() || romFile.fail()) {
        std::cerr << "failed to read ROM file '" << romFileName << "'\n";
        return ErrNoRomFileRead;
    }

    Disassembler disassembler(memory, sizeof(memory));
    std::cout << std::hex << std::setfill('0');

    while (disassembler.canDisassembleMore()) {
        auto address = disassembler.address();

        std::cout << "0x" << std::setw(4) << disassembler.address()
            << " : "
            << std::to_string(disassembler.nextMnemonic())
            << "            [";

        bool first = true;

        while (address < disassembler.address()) {
            if (first) {
                first = false;
            } else {
                std::cout << ' ';
            }

            std::cout << "0x" << static_cast<std::uint16_t>(*(memory + address));
            ++address;
        }

        std::cout << "]\n";
    }
}
