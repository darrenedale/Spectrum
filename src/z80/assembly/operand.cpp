//
// Created by darren on 17/03/2021.
//

#include <iostream>
#include <sstream>
#include <iomanip>
#include "operand.h"

using namespace Z80::Assembly;

std::string std::to_string(const Operand & op)
{
    std::ostringstream out;

    switch (op.mode) {
        case AddressingMode::Immediate:
            out << '$' << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(op.unsignedByte);
            break;

        case AddressingMode::ImmediateExtended:
        case AddressingMode::ModifiedPageZero:
            out << '$' << std::hex << std::setfill('0') << std::setw(4) << op.unsignedWord;
            break;

        case AddressingMode::Relative:
            if (0 > op.signedByte) {
                out << "$-" << std::hex << std::setw(0) << static_cast<std::uint16_t>(op.signedByte * -1);
            } else {
                out << "$+" << std::hex << std::setw(0) << static_cast<std::uint16_t>(op.signedByte);
            }
            break;

        case AddressingMode::Extended:
            out << "($" << std::hex << std::setfill('0') << std::setw(4) << op.unsignedWord << ')';
            break;

        case AddressingMode::Indexed:
            if (0 > op.signedByte) {
                out << '(' << to_string(op.indexedAddress.register16) << " - $" << std::hex << std::setw(0)
                    << static_cast<std::uint16_t>(op.indexedAddress.offset * -1) << ')';
            } else {
                out << '(' << to_string(op.indexedAddress.register16) << " + $" << std::hex << std::setw(0)
                    << static_cast<std::uint16_t>(op.indexedAddress.offset) << ')';
            }
            break;

        case AddressingMode::Register8:
            out << to_string(op.register8);
            break;

        case AddressingMode::Register16:
            out << to_string(op.register16);
            break;

        case AddressingMode::RegisterIndirect:
            out << '(' << to_string(op.register16) << ')';
            break;

        case AddressingMode::Bit:
            out << std::dec << std::setfill(' ') << std::setw(0) << op.bit;
            break;
    }

    return out.str();
}

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
    
