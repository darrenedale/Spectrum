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
//
//OperandValue Operand::evaluate(::Z80::Z80 * cpu, bool asDestination) const
//{
//    switch (mode) {
//        case AddressingMode::Immediate:
//            return {.unsignedByte = unsignedByte};
//
//        case AddressingMode::ImmediateExtended:
//            // the immediate 16-bit value
//        case AddressingMode::ModifiedPageZero:
//            // the RST address
//            return {.unsignedWord = unsignedWord};
//
//        case AddressingMode::Relative:
//            // the address calculated from the relative offset
//            return {.unsignedWord = static_cast<UnsignedWord>(cpu->pc() + signedByte)};
//
//        case AddressingMode::Extended:
//            if (asDestination) {
//                // the destination address
//                return {.unsignedWord = unsignedWord};
//            } else {
//                // the value at the source address
//                return {.unsignedWord = cpu->peekUnsignedWord(unsignedWord)};
//            }
//
//        case AddressingMode::Indexed:
//            if (asDestination) {
//                // the destination address
//                return {.unsignedWord = static_cast<UnsignedWord>(cpu->registerValue(indexedAddress.register16) +
//                                                                  indexedAddress.offset)};
//            } else {
//                // the value at the source address
//                return {.unsignedWord = cpu->peekUnsignedWord(static_cast<UnsignedWord>(cpu->registerValue(indexedAddress.register16) +
//                                                                  indexedAddress.offset))};
//            }
//
//        case AddressingMode::Register8:
//            // TODO for now we're just returning the register's content in all cases, but strictly speaking we should
//            //  return the register if it's as destination, the register value if it's as source
//            return {.unsignedByte = cpu->registerValue(register8)};
//
//        case AddressingMode::Register16:
//            // TODO for now we're just returning the register's content in all cases, but strictly speaking we should
//            //  return the register if it's as destination, the register value if it's as source
//            return {.unsignedWord = cpu->registerValue(register16)};
//
//        case AddressingMode::Register8Indirect:
//            // the port determined from the register
//            return {.unsignedWord = static_cast<UnsignedWord>((cpu->bRegisterValue() << 8) |
//                                                              cpu->registerValue(register8))};
//
//        case AddressingMode::Register16Indirect:
//        {
//            UnsignedWord addr = cpu->registerValue(register16);
//
//            if (asDestination) {
//                // address stored in 16-bit register
//                return {.unsignedWord = addr};
//            } else {
//                // value at address stored in 16-bit register
//                if (0xffff > addr) {
//                    return {.unsignedWord = cpu->peekUnsignedWord(cpu->registerValue(register16))};
//                } else {
//                    return {.unsignedWord = cpu->peekUnsigned(cpu->registerValue(register16))};
//                }
//            }
//        }
//
//        case AddressingMode::Bit:
//            return {.bit = bit};
//    }
//
//    return {};
//}
