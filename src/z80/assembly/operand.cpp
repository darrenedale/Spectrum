//
// Created by darren on 17/03/2021.
//

#include <iostream>
#include <sstream>
#include <iomanip>
#include "operand.h"
#include "../z80.h"

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

        case AddressingMode::Register8Indirect:
            out << '(' << to_string(op.register8) << ')';
            break;

        case AddressingMode::Register16Indirect:
            out << '(' << to_string(op.register16) << ')';
            break;

        case AddressingMode::Bit:
            out << std::dec << std::setfill(' ') << std::setw(0) << static_cast<std::uint16_t>(op.bit);
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

OperandValue Operand::evaluate(::Z80::Z80 * cpu, bool asDestination) const
{
    switch (mode) {
        case AddressingMode::Immediate:
            return {.unsignedByte = unsignedByte};

        case AddressingMode::ImmediateExtended:
            // the immediate 16-bit value
        case AddressingMode::ModifiedPageZero:
            // the RST address
            return {.unsignedWord = unsignedWord};

        case AddressingMode::Relative:
            // the address calculated from the relative offset
            return {.unsignedWord = static_cast<UnsignedWord>(cpu->pc() + signedByte)};

        case AddressingMode::Extended:
            if (asDestination) {
                // the destination address
                return {.unsignedWord = unsignedWord};
            } else {
                // the value at the source address
                return {.unsignedWord = cpu->peekUnsignedWord(unsignedWord)};
            }

        case AddressingMode::Indexed:
            if (asDestination) {
                // the destination address
                return {.unsignedWord = static_cast<UnsignedWord>(cpu->registerValue(indexedAddress.register16) +
                                                                  indexedAddress.offset)};
            } else {
                // the value at the source address
                return {.unsignedWord = cpu->peekUnsignedWord(static_cast<UnsignedWord>(cpu->registerValue(indexedAddress.register16) +
                                                                  indexedAddress.offset))};
            }

        case AddressingMode::Register8:
            // TODO for now we're just returning the register's content in all cases, but strictly speaking we should
            //  return the register if it's as destination, the register value if it's as source
            return {.unsignedByte = cpu->registerValue(register8)};

        case AddressingMode::Register16:
            // TODO for now we're just returning the register's content in all cases, but strictly speaking we should
            //  return the register if it's as destination, the register value if it's as source
            return {.unsignedWord = cpu->registerValue(register16)};

        case AddressingMode::Register8Indirect:
            // the port determined from the register
            return {.unsignedWord = static_cast<UnsignedWord>((cpu->bRegisterValue() << 8) |
                                                              cpu->registerValue(register8))};

        case AddressingMode::Register16Indirect:
        {
            UnsignedWord addr = cpu->registerValue(register16);

            if (asDestination) {
                // address stored in 16-bit register
                return {.unsignedWord = addr};
            } else {
                // value at address stored in 16-bit register
                if (0xffff > addr) {
                    return {.unsignedWord = cpu->peekUnsignedWord(cpu->registerValue(register16))};
                } else {
                    return {.unsignedWord = cpu->peekUnsigned(cpu->registerValue(register16))};
                }
            }
        }

        case AddressingMode::Bit:
            return {.bit = bit};
    }

    return {};
}
