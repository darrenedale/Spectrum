//
// Created by darren on 31/03/2021.
//

#include <iostream>
#include "types.h"

std::string std::to_string(const Z80::Register16 & reg)
{
    switch (reg) {
        case Z80::Register16::AF:
            return "AF";

        case Z80::Register16::BC:
            return "BC";

        case Z80::Register16::DE:
            return "DE";

        case Z80::Register16::HL:
            return "HL";

        case Z80::Register16::IX:
            return "IX";

        case Z80::Register16::IY:
            return "IY";

        case Z80::Register16::SP:
            return "SP";

        case Z80::Register16::PC:
            return "PC";

        case Z80::Register16::AFShadow:
            return "AF'";

        case Z80::Register16::BCShadow:
            return "BC'";

        case Z80::Register16::DEShadow:
            return "DE'";

        case Z80::Register16::HLShadow:
            return "HL'";
    }

    std::cerr << "unrecognised register pair (Register16 enumerator = " << static_cast<std::uint16_t>(reg) << ")\n";
    return {};
}

std::string std::to_string(const Z80::Register8 & reg)
{
    switch (reg) {
        case Z80::Register8::A:
            return "A";

        case Z80::Register8::F:
            return "F";

        case Z80::Register8::B:
            return "B";

        case Z80::Register8::C:
            return "C";

        case Z80::Register8::D:
            return "D";

        case Z80::Register8::E:
            return "E";

        case Z80::Register8::H:
            return "H";

        case Z80::Register8::L:
            return "L";

        case Z80::Register8::IXH:
            return "IXH";

        case Z80::Register8::IXL:
            return "IXL";

        case Z80::Register8::IYH:
            return "IYH";

        case Z80::Register8::IYL:
            return "IYL";

        case Z80::Register8::I:
            return "I";

        case Z80::Register8::R:
            return "R";

        case Z80::Register8::AShadow:
            return "A'";

        case Z80::Register8::FShadow:
            return "F'";

        case Z80::Register8::BShadow:
            return "B'";

        case Z80::Register8::CShadow:
            return "C'";

        case Z80::Register8::DShadow:
            return "D'";

        case Z80::Register8::EShadow:
            return "E'";

        case Z80::Register8::HShadow:
            return "H'";

        case Z80::Register8::LShadow:
            return "L'";
    }

    std::cerr << "unrecognised register (Register8 enumerator = " << static_cast<std::uint16_t>(reg) << ")\n";
    return {};
}
