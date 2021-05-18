//
// Created by darren on 26/03/2021.
//

#include "executionhistory.h"
#include "z80.h"

using ::Z80::Assembly::AddressingMode;
using ::Z80::Assembly::Operand;
using ::Z80::Assembly::OperandValue;

void Z80::ExecutedInstruction::readMemoryFragments(::Z80::Z80::Memory * memory)
{
    UnsignedWord address = ::Z80::z80ToHostByteOrder(*reinterpret_cast<UnsignedWord *>(machineCode + 1));

    if (address < 0xffff) {
        memory->readBytes(address, 2, memoryFragments.indirectPcPlus1);
    } else {
        *memoryFragments.indirectPcPlus1 = memory->readByte(0xffff);
        *(memoryFragments.indirectPcPlus1 + 1) = 0x00;
    }

    address = ::Z80::z80ToHostByteOrder(*reinterpret_cast<UnsignedWord *>(machineCode + 2));

    if (address < 0xffff) {
        memory->readBytes(address, 2, memoryFragments.indirectPcPlus2);
    } else {
        *memoryFragments.indirectPcPlus2 = memory->readByte(0xffff);
        *(memoryFragments.indirectPcPlus2 + 1) = 0x00;
    }

    if (registersBefore.hl < 0xffff) {
        memory->readBytes(registersBefore.hl, 2, memoryFragments.indirectHl);
    } else {
        *memoryFragments.indirectHl = memory->readByte(0xffff);
        *(memoryFragments.indirectHl + 1) = 0x00;
    }

    if (registersBefore.de < 0xffff) {
        memory->readBytes(registersBefore.de, 2, memoryFragments.indirectDe);
    } else {
        *memoryFragments.indirectDe = memory->readByte(0xffff);
        *(memoryFragments.indirectDe + 1) = 0x00;
    }

    if (registersBefore.bc < 0xffff) {
        memory->readBytes(registersBefore.bc, 2, memoryFragments.indirectBc);
    } else {
        *memoryFragments.indirectBc = memory->readByte(0xffff);
        *(memoryFragments.indirectBc + 1) = 0x00;
    }

    if (registersBefore.sp < 0xffff) {
        memory->readBytes(registersBefore.sp, 2, memoryFragments.indirectSp);
    } else {
        *memoryFragments.indirectSp = memory->readByte(0xffff);
        *(memoryFragments.indirectSp + 1) = 0x00;
    }

    auto offset = static_cast<SignedByte>(machineCode[2]);
    address = static_cast<UnsignedWord>(registersBefore.ix + offset);

    if (address < 0xffff) {
        memory->readBytes(address, 2, memoryFragments.indirectIxWithOffset);
    } else {
        *memoryFragments.indirectIxWithOffset = memory->readByte(0xffff);
        *(memoryFragments.indirectIxWithOffset + 1) = 0x00;
    }

    address = static_cast<UnsignedWord>(registersBefore.iy + offset);

    if (address < 0xffff) {
        memory->readBytes(address, 2, memoryFragments.indirectIyWithOffset);
    } else {
        *memoryFragments.indirectIyWithOffset = memory->readByte(0xffff);
        *(memoryFragments.indirectIyWithOffset + 1) = 0x00;
    }
}

/**
 * TODO we may need to do some conversion to host byte order (doesn't matter on x86 or x64, byte order is the same)
 *
 * @param operand
 * @param asDestination
 * @return
 */
OperandValue Z80::ExecutedInstruction::evaluateOperand(const Operand & operand, bool asDestination) const
{
    switch (operand.mode) {
        case AddressingMode::Immediate:
            return {.unsignedByte = operand.unsignedByte};

        case AddressingMode::ImmediateExtended:
            // the immediate 16-bit value
        case AddressingMode::ModifiedPageZero:
            // the RST address
            return {.unsignedWord = operand.unsignedWord};

        case AddressingMode::Relative:
            // the address calculated from the relative offset
            return {.unsignedWord = static_cast<UnsignedWord>(registersBefore.pc + operand.signedByte)};

        case AddressingMode::Extended:
            if (asDestination) {
                // the destination address
                return {.unsignedWord = operand.unsignedWord};
            } else {
                // the value at the source address
                if (0xcb == machineCode[0] || 0xed == machineCode[0] || 0xdd == machineCode[0] || 0xfd == machineCode[0]) {
                    // prefixed opcode, the extended address is at PC + 2
                    return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectPcPlus2)};
                } else {
                    // plain opcode, the extended address is at PC + 1
                    return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectPcPlus1)};
                }
            }

        case AddressingMode::Indexed:
            if (asDestination) {
                // the destination address
                if (Register16::IX == operand.indexedAddress.register16) {
                    return {.unsignedWord = static_cast<UnsignedWord>(registersBefore.ix + operand.indexedAddress.offset)};
                } else {
                    return {.unsignedWord = static_cast<UnsignedWord>(registersBefore.iy + operand.indexedAddress.offset)};
                }
            } else {
                // the value at the source address
                if (Register16::IX == operand.indexedAddress.register16) {
                    return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectIxWithOffset)};
                } else {
                    return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectIyWithOffset)};
                }
            }

        case AddressingMode::Register8:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // we're only interested in the 8-bit registers that can be addressed explicitly using register addressing
            // TODO for now we're just returning the register's content in all cases, but strictly speaking we should
            //  return the register if it's as destination, the register value if it's as source
            switch (operand.register8) {
                case Register8::A:
                    return {.unsignedByte = registersBefore.a};

                case Register8::F:
                    return {.unsignedByte = registersBefore.f};

                case Register8::B:
                    return {.unsignedByte = registersBefore.b};

                case Register8::C:
                    return {.unsignedByte = registersBefore.c};

                case Register8::D:
                    return {.unsignedByte = registersBefore.d};

                case Register8::E:
                    return {.unsignedByte = registersBefore.e};

                case Register8::H:
                    return {.unsignedByte = registersBefore.h};

                case Register8::L:
                    return {.unsignedByte = registersBefore.l};

                case Register8::I:
                    return {.unsignedByte = registersBefore.i};

                case Register8::R:
                    return {.unsignedByte = registersBefore.r};
            }
#pragma clang diagnostic pop

        case AddressingMode::Register16:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // we're only interested in the 16-bit registers that can be addressed explicitly using register addressing
            // TODO for now we're just returning the register's content in all cases, but strictly speaking we should
            //  return the register if it's as destination, the register value if it's as source
            switch (operand.register16) {
                case Register16::AF:
                    return {.unsignedWord = registersBefore.af};

                case Register16::BC:
                    return {.unsignedWord = registersBefore.bc};

                case Register16::DE:
                    return {.unsignedWord = registersBefore.de};

                case Register16::HL:
                    return {.unsignedWord = registersBefore.hl};

                case Register16::IX:
                    return {.unsignedWord = registersBefore.ix};

                case Register16::IY:
                    return {.unsignedWord = registersBefore.iy};
           }
#pragma clang diagnostic pop

        case AddressingMode::Register8Indirect: {
            // the port determined from the register
            UnsignedByte registerValue = 0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // we're only interested in the 8-bit registers that can be addressed explicitly using indirect addressing
            switch (operand.register8) {
                case Register8::A:
                    registerValue = registersBefore.a;

                case Register8::F:
                    registerValue = registersBefore.f;

                case Register8::B:
                    registerValue = registersBefore.b;

                case Register8::C:
                    registerValue = registersBefore.c;

                case Register8::D:
                    registerValue = registersBefore.d;

                case Register8::E:
                    registerValue = registersBefore.e;

                case Register8::H:
                    registerValue = registersBefore.h;

                case Register8::L:
                    registerValue = registersBefore.l;

                case Register8::I:
                    registerValue = registersBefore.i;

                case Register8::R:
                    registerValue = registersBefore.r;
            }
#pragma clang diagnostic pop

            return {.unsignedWord = static_cast<UnsignedWord>((registersBefore.b << 8) | registerValue)};
        }

        case AddressingMode::Register16Indirect: {
            if (asDestination) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // we're only interested in the 16-bit registers that can be addressed explicitly as destinations using register
                                                // indirect addressing
                switch (operand.register16) {
                    case Register16::AF:
                        return {.unsignedWord = registersBefore.af};

                    case Register16::BC:
                        return {.unsignedWord = registersBefore.bc};

                    case Register16::DE:
                        return {.unsignedWord = registersBefore.de};

                    case Register16::HL:
                        return {.unsignedWord = registersBefore.hl};

                    case Register16::IX:
                        return {.unsignedWord = registersBefore.ix};

                    case Register16::IY:
                        return {.unsignedWord = registersBefore.iy};
                }
#pragma clang diagnostic pop
            } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"     // we're only interested in the 16-bit registers that can be addressed explicitly as sources using register
                                                // indirect addressing
                switch (operand.register16) {
                    case Register16::BC:
                        return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectBc)};

                    case Register16::DE:
                        return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectDe)};

                    case Register16::HL:
                        return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectHl)};

                    case Register16::SP:
                        return {.unsignedWord = *reinterpret_cast<const UnsignedWord *>(memoryFragments.indirectSp)};
                }
#pragma clang diagnostic pop
            }
        }
        break;

        case AddressingMode::Bit:
            return {.bit = operand.bit};
    }

    return {};
}

Z80::ExecutedInstruction::ExecutedInstruction(const UnsignedByte * instruction, ::Z80::Z80 * z80)
        : ExecutedInstruction()
{
    *reinterpret_cast<std::uint32_t *>(machineCode) = *reinterpret_cast<const std::uint32_t *>(instruction);
    registersBefore = z80->registers();
    readMemoryFragments(z80->memory());
}
