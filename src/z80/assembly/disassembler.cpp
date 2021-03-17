//
// Created by darren on 17/03/2021.
//

#include <iostream>
#include <iomanip>
#include <cstring>

#include "disassembler.h"
#include "../opcodes.h"
#include "../z80.h"

using namespace Z80::Assembly;

using Register8 = ::Z80::Register8;
using Register16 = ::Z80::Register16;
using UnsignedWord = ::Z80::UnsignedWord;
using UnsignedByte = ::Z80::UnsignedByte;
using SignedByte = ::Z80::SignedByte;

namespace
{
    UnsignedWord readUnsignedWord(const Z80::UnsignedByte * memory)
    {
        return (static_cast<UnsignedWord>(*memory) & 0x00ff) | ((static_cast<UnsignedWord>(*(memory + 1)) & 0x00ff) << 8);
    }
}

Disassembler::Mnemonics Disassembler::disassembleFrom(int address) const
{
    static UnsignedByte overflowBuffer[4];
    Mnemonics ret;

    while (address < m_memorySize) {
        UnsignedByte * machineCode = m_memory + address;
        auto bytesAvailable = m_memorySize - address;

        if (bytesAvailable < 4) {
            std::memcpy(overflowBuffer, machineCode, bytesAvailable);
            std::memcpy(overflowBuffer + bytesAvailable, m_memory, 4 - bytesAvailable);
            machineCode = overflowBuffer;
        }

        auto mnemonic = disassembleOne(machineCode);
        address += mnemonic.size;
        ret.push_back(std::move(mnemonic));
    }

    return ret;
}

Mnemonic Disassembler::nextMnemonic()
{
    if (m_pc >= m_memorySize) {
        return {
            Instruction::NOP,
            {},
            1,
        };
    }

    auto * machineCode = m_memory + m_pc;
    UnsignedByte overflowBuffer[4];
    auto bytesAvailable = m_memorySize - m_pc;

    if (bytesAvailable < 4) {
        std::memcpy(overflowBuffer, machineCode, bytesAvailable);
        std::memcpy(overflowBuffer + bytesAvailable, m_memory, 4 - bytesAvailable);
        machineCode = overflowBuffer;
    }

    auto mnemonic = disassembleOne(machineCode);
    m_pc += mnemonic.size;
    return mnemonic;
}

Mnemonic Disassembler::disassembleOne(const Z80::UnsignedByte * machineCode)
{
    switch (*machineCode) {
        case 0xcb:
            return disassembleOneCb(machineCode + 1);

        case 0xed:
            return disassembleOneEd(machineCode + 1);

        case 0xdd:
            return disassembleOneDdOrFd(Register16::IX, machineCode + 1);

        case 0xfd:
            return disassembleOneDdOrFd(Register16::IY, machineCode + 1);

        default:
            return disassembleOnePlain(machineCode);
    }
}

Mnemonic Disassembler::disassembleOnePlain(const Z80::UnsignedByte * machineCode)
{
    switch (*machineCode) {
        case Z80__PLAIN__NOP:                // 0x00
            return {
                Instruction::NOP,
                {},
                1,
            };

        case Z80__PLAIN__LD__BC__NN:         // 0x01
            return {
                Instruction::LD,
                {
                        {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                        {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3,
            };
        
        case Z80__PLAIN__LD__INDIRECT_BC__A:                // 0x02
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::BC,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                     },
                    1,
            };
        
        case Z80__PLAIN__INC__BC:                // 0x03
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                     },
                    1,
            };

        case Z80__PLAIN__INC__B:                // 0x04
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    1,
            };

        case Z80__PLAIN__DEC__B:                // 0x05
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__B__N:                // 0x06
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    2,
            };

        case Z80__PLAIN__RLCA:                // 0x07
            return {
                    Instruction::RLCA,
                    {},
                    1,
            };

        case Z80__PLAIN__EX__AF__AF_SHADOW:                // 0x08
            return {
                    Instruction::EX,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::AF,},
                             {.mode = AddressingMode::Register16, .register16 = Register16::AFShadow,},
                     },
                    1,
            };

        case Z80__PLAIN__ADD__HL__BC:                // 0x09
            return {
                    Instruction::ADD,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__A__INDIRECT_BC:                // 0x0a
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                             {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::BC,},
                     },
                    1,
            };

        case Z80__PLAIN__DEC__BC:                // 0x0b
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                     },
                    1,
            };

        case Z80__PLAIN__INC__C:                // 0x0c
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                     },
                    1,
            };

        case Z80__PLAIN__DEC__C:                // 0x0d
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__C__N:                // 0x0e
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    2,
            };

        case Z80__PLAIN__RRCA:                // 0x0f
            return {
                    Instruction::RRCA,
                    {},
                    1,
            };

        case Z80__PLAIN__DJNZ__d:                // 0x10
            return {
                Instruction::DJNZ,
                {
                        {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),}
                },
                1,
            };

        case Z80__PLAIN__LD__DE__NN:                // 0x11
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                             {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                     },
                    3,
            };

        case Z80__PLAIN__LD__INDIRECT_DE__A:                // 0x12
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::DE,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                     },
                    1,
            };

        case Z80__PLAIN__INC__DE:                // 0x13
            return {
                    Instruction::INC,
                    {
                            { .mode = AddressingMode::Register16, .register16 = Register16::DE,}
                    },
                    1,
            };

        case Z80__PLAIN__INC__D:                // 0x14
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                     },
                    1,
            };

        case Z80__PLAIN__DEC__D:                // 0x15
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__D__N:                // 0x16
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    2,
            };

        case Z80__PLAIN__RLA:                // 0x17
            return {
                Instruction::RLA,
                {},
                1,
            };

        case Z80__PLAIN__JR__d:                // 0x18
            return {
                Instruction::JR,
                {
                        {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                },
                2
            };

        case Z80__PLAIN__ADD__HL__DE:                // 0x19
            return {
                    Instruction::ADD,
                    {
                            { .mode = AddressingMode::Register16, .register16 =Register16::HL,},
                            { .mode = AddressingMode::Register16, .register16 =Register16::DE,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__INDIRECT_DE:                // 0x1a
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 =Register8::A,},
                            { .mode = AddressingMode::RegisterIndirect, .register16 =Register16::DE,},
                    },
                    1,
            };

        case Z80__PLAIN__DEC__DE:                // 0x1b
            return {
                Instruction::DEC,
                {
                    { .mode = AddressingMode::Register16, .register16 = Register16::DE,}
                },
                1,
            };

        case Z80__PLAIN__INC__E:                // 0x1c
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                     },
                    1,
            };

        case Z80__PLAIN__DEC__E:                // 0x1d
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__E__N:                // 0x1e
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    2,
            };

        case Z80__PLAIN__RRA:                // 0x1f
            return {
                Instruction::RRA,
                {},
                1,
            };

        case Z80__PLAIN__JR__NZ__d:                // 0x20
            return {
                    Instruction::JRNZ,
                    {
                            {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                    },
                    2
            };

        case Z80__PLAIN__LD__HL__NN:                // 0x21
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register16, .register16 = Register16::HL,},
                            { .mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),}
                    },
                    3,
            };

        case Z80__PLAIN__LD__INDIRECT_NN__HL:                // 0x22
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    3,
            };

        case Z80__PLAIN__INC__HL:                // 0x23
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                     },
                    1,
            };

        case Z80__PLAIN__INC__H:                // 0x24
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                     },
                    1,
            };

        case Z80__PLAIN__DEC__H:                // 0x25
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__H__N:                // 0x26
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    2,
            };

        case Z80__PLAIN__DAA:                // 0x27
            return {
                Instruction::DAA,
                {},
                1,
            };
            
        case Z80__PLAIN__JR__Z__d:                // 0x28
            return {
                    Instruction::JRZ,
                    {
                            {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                    },
                    2
            };

        case Z80__PLAIN__ADD__HL__HL:                // 0x29
            return {
                    Instruction::ADD,
                    {
                            { .mode = AddressingMode::Register16, .register16 =Register16::HL,},
                            { .mode = AddressingMode::Register16, .register16 =Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__HL__INDIRECT_NN:                // 0x2a
            return {
                Instruction::LD,
                {
                        {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                        {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3,
            };

        case Z80__PLAIN__DEC__HL:                // 0x2b
            return {
                    Instruction::DEC,
                    {
                            { .mode = AddressingMode::Register16, .register16 = Register16::HL,}
                    },
                    1,
            };
            
        case Z80__PLAIN__INC__L:                // 0x2c
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                     },
                    1,
            };
            
        case Z80__PLAIN__DEC__L:                // 0x2d
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__L__N:                // 0x2e
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    2,
            };

        case Z80__PLAIN__CPL:                // 0x2f
            return {
                Instruction::CPL,
                {},
                1,
            };
            
        case Z80__PLAIN__JR__NC__d:                // 0x30
            return {
                Instruction::JRNC,
                {
                        {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)), },
                },
                2,
            };

        case Z80__PLAIN__LD__SP__NN:                // 0x31
            return {
                Instruction::LD,
                {
                    { .mode = AddressingMode::Register16, .register16 = Register16::SP,},
                    { .mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                1,
            };

        case Z80__PLAIN__LD__INDIRECT_NN__A:                // 0x32
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    3,
            };

        case Z80__PLAIN__INC__SP:                // 0x33
            return {
                    Instruction::INC,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::SP,},
                     },
                    1,
            };

        case Z80__PLAIN__INC__INDIRECT_HL:                // 0x34
            return {
                    Instruction::INC,
                    {
                            { .mode = AddressingMode::RegisterIndirect, .register16 =Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__DEC__INDIRECT_HL:                // 0x35
            return {
                    Instruction::DEC,
                    {
                            { .mode = AddressingMode::RegisterIndirect, .register16 =Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__N:                // 0x36
            return {
                Instruction::LD,
                {
                    { .mode = AddressingMode::RegisterIndirect, .register16 =Register16::HL,},
                    { .mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                },
                2,
            };

        case Z80__PLAIN__SCF:                // 0x37
            return {
                Instruction::SCF,
                {},
                1
            };

        case Z80__PLAIN__JR__C__d:                // 0x38
            return {
                    Instruction::JRC,
                    {
                            {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                    },
                    2
            };

        case Z80__PLAIN__ADD__HL__SP:                // 0x39
            return {
                    Instruction::ADD,
                    {
                             {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register16, .register16 = Register16::SP,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__A__INDIRECT_NN:                // 0x3a
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3,
            };

        case Z80__PLAIN__DEC__SP:                // 0x3b
            return {
                    Instruction::DEC,
                    {
                            { .mode = AddressingMode::Register16, .register16 = Register16::SP,}
                    },
                    1,
            };

        case Z80__PLAIN__INC__A:                // 0x3c
            return {
                Instruction::INC,
                {
                 {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                1,
            };

        case Z80__PLAIN__DEC__A:                // 0x3d
            return {
                    Instruction::DEC,
                    {
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__A__N:                // 0x3e
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 =Register8::A,},
                            { .mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2,
            };

        case Z80__PLAIN__CCF:                // 0x3f
            return {
                Instruction::CCF,
                {},
                1,
            };

        case Z80__PLAIN__LD__B__B:                // 0x40
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__C:                // 0x41
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__D:                // 0x42
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__E:                // 0x43
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__H:                // 0x44
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__L:                // 0x45
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__INDIRECT_HL:                // 0x46
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__B__A:                // 0x47
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__B:                // 0x48
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__C:                // 0x49
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__D:                // 0x4a
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__E:                // 0x4b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__H:                // 0x4c
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__L:                // 0x4d
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__INDIRECT_HL:                // 0x4e
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__C__A:                // 0x4f
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__B:                // 0x50
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__C:                // 0x51
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__D:                // 0x52
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__E:                // 0x53
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__H:                // 0x54
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__L:                // 0x55
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__INDIRECT_HL:                // 0x56
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__D__A:                // 0x57
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__B:                // 0x58
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__C:                // 0x59
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__D:                // 0x5a
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__E:                // 0x5b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__H:                // 0x5c
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__L:                // 0x5d
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__INDIRECT_HL:                // 0x5e
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__E__A:                // 0x5f
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__B:                // 0x60
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__C:                // 0x61
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__D:                // 0x62
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__E:                // 0x63
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__H:                // 0x64
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__L:                // 0x65
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__INDIRECT_HL:                // 0x66
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__H__A:                // 0x67
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__B:                // 0x68
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__C:                // 0x69
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__D:                // 0x6a
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__E:                // 0x6b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__H:                // 0x6c
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__L:                // 0x6d
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__INDIRECT_HL:                // 0x6e
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__L__A:                // 0x6f
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__B:                // 0x70
            return {
                    Instruction::LD,
                    {
                             {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__C:                // 0x71
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__D:                // 0x72
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__E:                // 0x73
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__H:                // 0x74
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__L:                // 0x75
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__HALT:                // 0x76
            return {
                Instruction::HALT,
                {},
                1
            };

        case Z80__PLAIN__LD__INDIRECT_HL__A:                // 0x77
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__B:                // 0x78
            return {
                Instruction::LD,
                {
                        {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                        {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                1,
            };

        case Z80__PLAIN__LD__A__C:                // 0x79
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__D:                // 0x7a
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__E:                // 0x7b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__H:                // 0x7c
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__L:                // 0x7d
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__INDIRECT_HL:                // 0x7e
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__A__A:                // 0x7f
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__B:                // 0x80
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__C:                // 0x81
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__D:                // 0x82
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__E:                // 0x83
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__H:                // 0x84
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__L:                // 0x85
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__INDIRECT_HL:                // 0x86
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__A:                // 0x87
            return {
                    Instruction::ADD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__B:                // 0x88
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__C:                // 0x89
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__D:                // 0x8a
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__E:                // 0x8b
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__H:                // 0x8c
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__L:                // 0x8d
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__INDIRECT_HL:                // 0x8e
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__ADC__A__A:                // 0x8f
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__B:                // 0x90
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__C:                // 0x91
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__D:                // 0x92
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__E:                // 0x93
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__H:                // 0x94
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__L:                // 0x95
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__INDIRECT_HL:                // 0x96
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__SUB__A:                // 0x97
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__B:                // 0x98
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__C:                // 0x99
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__D:                // 0x9a
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__E:                // 0x9b
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__H:                // 0x9c
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__L:                // 0x9d
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__INDIRECT_HL:                // 0x9e
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__SBC__A__A:                // 0x9f
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    1,
            };

        case Z80__PLAIN__AND__B:                // 0xa0
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    1,
            };

        case Z80__PLAIN__AND__C:                // 0xa1
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    1,
            };

        case Z80__PLAIN__AND__D:                // 0xa2
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    1,
            };

        case Z80__PLAIN__AND__E:                // 0xa3
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    1,
            };

        case Z80__PLAIN__AND__H:                // 0xa4
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    1,
            };

        case Z80__PLAIN__AND__L:                // 0xa5
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    1,
            };

        case Z80__PLAIN__AND__INDIRECT_HL:                // 0xa6
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__AND__A:                // 0xa7
            return {
                Instruction::AND,
                {
                        {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                },
                1,
            };

        case Z80__PLAIN__XOR__B:                // 0xa8
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    1,
            };

        case Z80__PLAIN__XOR__C:                // 0xa9
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    1,
            };

        case Z80__PLAIN__XOR__D:                // 0xaa
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    1,
            };

        case Z80__PLAIN__XOR__E:                // 0xab
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    1,
            };

        case Z80__PLAIN__XOR__H:                // 0xac
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    1,
            };

        case Z80__PLAIN__XOR__L:                // 0xad
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    1,
            };

        case Z80__PLAIN__XOR__INDIRECT_HL:                // 0xae
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__XOR__A:                // 0xaf
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__B:                // 0xb0
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__C:                // 0xb1
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__D:                // 0xb2
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__E:                // 0xb3
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__H:                // 0xb4
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__L:                // 0xb5
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    1,
            };

        case Z80__PLAIN__OR__INDIRECT_HL:                // 0xb6
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__OR__A:                // 0xb7
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__B:                // 0xb8
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__C:                // 0xb9
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__D:                // 0xba
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__E:                // 0xbb
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__H:                // 0xbc
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__L:                // 0xbd
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    1,
            };

        case Z80__PLAIN__CP__INDIRECT_HL:                // 0xbe
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__CP__A:                // 0xbf
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                    },
                    1,
            };

        case Z80__PLAIN__RET__NZ:                // 0xc0
            return {
                Instruction::RETNZ,
                {},
                1
            };

        case Z80__PLAIN__POP__BC:                // 0xc1
            return {
                Instruction::POP,
                {
                     {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                },
                1,
            };

        case Z80__PLAIN__JP__NZ__NN:                // 0xc2
            return {
                    Instruction::JPNZ,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__JP__NN:                // 0xc3
            return {
                    Instruction::JP,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord((machineCode + 1)), },
                    },
                    3,
            };

        case Z80__PLAIN__CALL__NZ__NN:                // 0xc4
            return {
                    Instruction::CALLNZ,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__PUSH__BC:                // 0xc5
            return {
                    Instruction::PUSH,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                    },
                    1,
            };

        case Z80__PLAIN__ADD__A__N:                // 0xc6
            return {
                Instruction::ADD,
                {
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                },
                2
            };

        case Z80__PLAIN__RST__00:                // 0xc7
            return {
                    Instruction::RST,
                    {
                        // NOTE no need for endian conversion, both bytes are the same
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = 0x0000,},
                    },
                    1,
            };

        case Z80__PLAIN__RET__Z:                // 0xc8
            return {
                Instruction::RETZ,
                {},
                1
            };

        case Z80__PLAIN__RET:                // 0xc9
            return {
                    Instruction::RET,
                    {},
                    1
            };

        case Z80__PLAIN__JP__Z__NN:                // 0xca
            return {
                    Instruction::JPZ,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__PREFIX__CB:                // 0xcb
            // NOTE should never get here
            return disassembleOneCb(machineCode + 1);

        case Z80__PLAIN__CALL__Z__NN:                // 0xcc
            return {
                    Instruction::CALLZ,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__CALL__NN:                // 0xcd
            return {
                Instruction::CALL,
                {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3,
            };

        case Z80__PLAIN__ADC__A__N:                // 0xce
            return {
                Instruction::ADC,
                {
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                },
                2
            };

        case Z80__PLAIN__RST__08:                // 0xcf
            return {
                    Instruction::RST,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0008),},
                    },
                    1,
            };

        case Z80__PLAIN__RET__NC:                // 0xd0
            return {
                    Instruction::RETNC,
                    {},
                    1
            };

        case Z80__PLAIN__POP__DE:                // 0xd1
            return {
                Instruction::POP,
                {
                     {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                },
                1,
            };

        case Z80__PLAIN__JP__NC__NN:                // 0xd2
            return {
                    Instruction::JPNC,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__OUT__INDIRECT_N__A:                // 0xd3
            return {
                Instruction::OUT,
                {
                    {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                2
            };

        case Z80__PLAIN__CALL__NC__NN:                // 0xd4
            return {
                Instruction::CALLNC,
                {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3
            };

        case Z80__PLAIN__PUSH__DE:                // 0xd5
            return {
                Instruction::PUSH,
                {
                    {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                },
                1,
            };

        case Z80__PLAIN__SUB__N:                // 0xd6
            return {
                    Instruction::SUB,
                    {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };

        case Z80__PLAIN__RST__10:                // 0xd7
            return {
                    Instruction::RST,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0010),},
                    },
                    1,
            };

        case Z80__PLAIN__RET__C:                // 0xd8
            return {
                    Instruction::RETC,
                    {},
                    1
            };

        case Z80__PLAIN__EXX:                // 0xd9
            return {
                Instruction::EXX,
                {},
                1,
            };

        case Z80__PLAIN__JP__C__NN:                // 0xda
            return {
                    Instruction::JPC,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };
        
        case Z80__PLAIN__IN__A__INDIRECT_N:                // 0xdb
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };
            
        case Z80__PLAIN__CALL__C__NN:                // 0xdc
            return {
                    Instruction::CALLC,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };
            
        case Z80__PLAIN__PREFIX__DD:                // 0xdd
            // NOTE should never get here
            return disassembleOneDdOrFd(Register16::IX, machineCode + 1);
            
        case Z80__PLAIN__SBC__A__N:                // 0xde
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };
            
        case Z80__PLAIN__RST__18:                // 0xdf
            return {
                    Instruction::RST,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0018),},
                    },
                    1,
            };
            
        case Z80__PLAIN__RET__PO:                // 0xe0
            return {
                    Instruction::RETPO,
                    {},
                    1
            };

        case Z80__PLAIN__POP__HL:                // 0xe1
            return {
                Instruction::POP,
                {
                    {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                },
                1,
            };

        case Z80__PLAIN__JP__PO__NN:                // 0xe2
            return {
                    Instruction::JPPO,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__EX__INDIRECT_SP__HL:                // 0xe3
            return {
                    Instruction::EX,
                    {
                            {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::SP,},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    1
            };

        case Z80__PLAIN__CALL__PO__NN:                // 0xe4
            return {
                    Instruction::CALLPO,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };

        case Z80__PLAIN__PUSH__HL:                // 0xe5
            return {
                Instruction::PUSH,
                {
                    {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                },
                1,
            };

        case Z80__PLAIN__AND__N:                // 0xe6
            return {
                    Instruction::AND,
                    {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };

        case Z80__PLAIN__RST__20:                // 0xe7
            return {
                    Instruction::RST,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0020),},
                    },
                    1,
            };

        case Z80__PLAIN__RET__PE:                // 0xe8
            return {
                    Instruction::RETPE,
                    {},
                    1
            };

        case Z80__PLAIN__JP__INDIRECT_HL:                // 0xe9
            return {
                Instruction::JPM,
                {
                    {.mode = AddressingMode::RegisterIndirect, .register16 = Register16::HL,},
                },
                1
            };

        case Z80__PLAIN__JP__PE__NN:                // 0xea
            return {
                Instruction::JPPE,
                {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3
            };
            
        case Z80__PLAIN__EX__DE__HL:                // 0xeb
            return {
                    Instruction::EX,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    1
            };
        
        case Z80__PLAIN__CALL__PE__NN:                // 0xec
            return {
                Instruction::CALLPE,
                {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3
            };
            
        case Z80__PLAIN__PREFIX__ED:                // 0xed
            // NOTE should never get here
            return disassembleOneEd(machineCode + 1);
            
        case Z80__PLAIN__XOR__N:                // 0xee
            return {
                    Instruction::XOR,
                    {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };
            
        case Z80__PLAIN__RST__28:                // 0xef
            return {
                    Instruction::RST,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0028),},
                    },
                    1,
            };
        
        case Z80__PLAIN__RET__P:                // 0xf0
            return {
                    Instruction::RETP,
                    {},
                    1
            };
        
        case Z80__PLAIN__POP__AF:                // 0xf1
            return {
                Instruction::POP,
                {
                    {.mode = AddressingMode::Register16, .register16 = Register16::AF,},
                },
                1,
            };
        
        case Z80__PLAIN__JP__P__NN:                // 0xf2
            return {
                Instruction::JPP,
                {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3
            };
            
        case Z80__PLAIN__DI:                // 0xf3
            return {
                Instruction::DI,
                {},
                1
            };
            
        case Z80__PLAIN__CALL__P__NN:                // 0xf4
            return {
                    Instruction::CALLP,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    3
            };
            
        case Z80__PLAIN__PUSH__AF:                // 0xf5
            return {
                    Instruction::PUSH,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::AF,},
                    },
                    1,
            };
            
        case Z80__PLAIN__OR__N:                // 0xf6
            return {
                    Instruction::OR,
                    {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };
            
        case Z80__PLAIN__RST__30:                // 0xf7
            return {
                    Instruction::RST,
                    {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0030),},
                    },
                    1,
            };
            
        case Z80__PLAIN__RET__M:                // 0xf8
            return {
                Instruction::RETM,
                {},
                1
            };

        case Z80__PLAIN__LD__SP__HL:                // 0xf9
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP,},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    1
            };
            
        case Z80__PLAIN__JP__M__NN:                // 0xfa
            return {
                Instruction::JPM,
                {
                        {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                3
            };
            
        case Z80__PLAIN__EI:                // 0xfb
            return {
                Instruction::EI,
                {},
                1,
            };
            
        case Z80__PLAIN__CALL__M__NN:                // 0xfc
            return {
                Instruction::CALLM,
                {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                3
            };
            
        case Z80__PLAIN__PREFIX__FD:                // 0xfd
            // NOTE should never get here
            return disassembleOneDdOrFd(Register16::IY, machineCode + 1);
            
        case Z80__PLAIN__CP__N:                // 0xfe
            return {
                    Instruction::CP,
                    {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    2
            };
            
        case Z80__PLAIN__RST__38:                // 0xff
            return {
                Instruction::RST,
                {
                        {.mode = AddressingMode::ImmediateExtended, .unsignedWord = Z80::Z80::hostToZ80ByteOrder(0x0038),},
                },
                1,
            };
    }

    std::cerr << "disassembly of opcode 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
        Instruction::NOP,
        {},
        1,
    };
}

Mnemonic Disassembler::disassembleOneCb(const Z80::UnsignedByte * machineCode)
{
    // NOTE all 0xcb prefix opcodes are this size
    static constexpr const UnsignedByte OpcodeSize = 2;

    switch (*machineCode) {
        case Z80__CB__RLC__B:					// 0x00
            return {};

        case Z80__CB__RLC__C:					// 0x01
            return {};

        case Z80__CB__RLC__D:					// 0x02
            return {};

        case Z80__CB__RLC__E:					// 0x03
            return {};

        case Z80__CB__RLC__H:					// 0x04
            return {};

        case Z80__CB__RLC__L:					// 0x05
            return {};

        case Z80__CB__RLC__INDIRECT_HL:	// 0x06
            return {};

        case Z80__CB__RLC__A:					// 0x07

            return {};

        case Z80__CB__RRC__B:					// 0x08
            return {};

        case Z80__CB__RRC__C:					// 0x09
            return {};

        case Z80__CB__RRC__D:					// 0x0a
            return {};

        case Z80__CB__RRC__E:					// 0x0b
            return {};

        case Z80__CB__RRC__H:					// 0x0c
            return {};

        case Z80__CB__RRC__L:					// 0x0d
            return {};

        case Z80__CB__RRC__INDIRECT_HL:	// 0x0e
            return {};

        case Z80__CB__RRC__A:					// 0x0f

            return {};

        case Z80__CB__RL__B:					// 0x10
            return {};

        case Z80__CB__RL__C:					// 0x11
            return {
                Instruction::RL,
                {
                        {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                },
                OpcodeSize,
            };

        case Z80__CB__RL__D:					// 0x12
            return {};

        case Z80__CB__RL__E:					// 0x13
            return {};

        case Z80__CB__RL__H:					// 0x14
            return {};

        case Z80__CB__RL__L:					// 0x15
            return {};

        case Z80__CB__RL__INDIRECT_HL:		// 0x16
            return {};

        case Z80__CB__RL__A:					// 0x17

            return {};

        case Z80__CB__RR__B:					// 0x18
            return {};

        case Z80__CB__RR__C:					// 0x19
            return {};

        case Z80__CB__RR__D:					// 0x1a
            return {};

        case Z80__CB__RR__E:					// 0x1b
            return {};

        case Z80__CB__RR__H:					// 0x1c
            return {};

        case Z80__CB__RR__L:					// 0x1d
            return {};

        case Z80__CB__RR__INDIRECT_HL:		// 0x1e
            return {};

        case Z80__CB__RR__A:					// 0x1f

            return {};

        case Z80__CB__SLA__B:					// 0x20
            return {};

        case Z80__CB__SLA__C:					// 0x21
            return {};

        case Z80__CB__SLA__D:					// 0x22
            return {};

        case Z80__CB__SLA__E:					// 0x23
            return {};

        case Z80__CB__SLA__H:					// 0x24
            return {};

        case Z80__CB__SLA__L:					// 0x25
            return {};

        case Z80__CB__SLA__INDIRECT_HL:	// 0x26
            return {};

        case Z80__CB__SLA__A:					// 0x27

            return {};

        case Z80__CB__SRA__B:					// 0x28
            return {};

        case Z80__CB__SRA__C:					// 0x29
            return {};

        case Z80__CB__SRA__D:					// 0x2a
            return {};

        case Z80__CB__SRA__E:					// 0x2b
            return {};

        case Z80__CB__SRA__H:					// 0x2c
            return {};

        case Z80__CB__SRA__L:					// 0x2d
            return {};

        case Z80__CB__SRA__INDIRECT_HL:	// 0x2e
            return {};

        case Z80__CB__SRA__A:					// 0x2f

            return {};

        case Z80__CB__SLL__B:					// 0x30
            return {};

        case Z80__CB__SLL__C:					// 0x31
            return {};

        case Z80__CB__SLL__D:					// 0x32
            return {};

        case Z80__CB__SLL__E:					// 0x33
            return {};

        case Z80__CB__SLL__H:					// 0x34
            return {};

        case Z80__CB__SLL__L:					// 0x35
            return {};

        case Z80__CB__SLL__INDIRECT_HL:	// 0x36
            return {};

        case Z80__CB__SLL__A:					// 0x37

            return {};

        case Z80__CB__SRL__B:					// 0x38
            return {};

        case Z80__CB__SRL__C:					// 0x39
            return {};

        case Z80__CB__SRL__D:					// 0x3a
            return {};

        case Z80__CB__SRL__E:					// 0x3b
            return {};

        case Z80__CB__SRL__H:					// 0x3c
            return {};

        case Z80__CB__SRL__L:					// 0x3d
            return {};

        case Z80__CB__SRL__INDIRECT_HL:	// 0x3e
            return {};

        case Z80__CB__SRL__A:					// 0x3f

            return {};

        case Z80__CB__BIT__0__B:					// 0x40
            return {};

        case Z80__CB__BIT__0__C:					// 0x41
            return {};

        case Z80__CB__BIT__0__D:					// 0x42
            return {};

        case Z80__CB__BIT__0__E:					// 0x43
            return {};

        case Z80__CB__BIT__0__H:					// 0x44
            return {};

        case Z80__CB__BIT__0__L:					// 0x45
            return {};

        case Z80__CB__BIT__0__INDIRECT_HL:	// 0x46
            return {};

        case Z80__CB__BIT__0__A:					// 0x47

            return {};

        case Z80__CB__BIT__1__B:					// 0x48
            return {};

        case Z80__CB__BIT__1__C:					// 0x49
            return {};

        case Z80__CB__BIT__1__D:					// 0x4a
            return {};

        case Z80__CB__BIT__1__E:					// 0x4b
            return {};

        case Z80__CB__BIT__1__H:					// 0x4c
            return {};

        case Z80__CB__BIT__1__L:					// 0x4d
            return {};

        case Z80__CB__BIT__1__INDIRECT_HL:	// 0x4e
            return {};

        case Z80__CB__BIT__1__A:					// 0x4f

            return {};

        case Z80__CB__BIT__2__B:					// 0x50
            return {};

        case Z80__CB__BIT__2__C:					// 0x51
            return {};

        case Z80__CB__BIT__2__D:					// 0x52
            return {};

        case Z80__CB__BIT__2__E:					// 0x53
            return {};

        case Z80__CB__BIT__2__H:					// 0x54
            return {};

        case Z80__CB__BIT__2__L:					// 0x55
            return {};

        case Z80__CB__BIT__2__INDIRECT_HL:	// 0x56
            return {};

        case Z80__CB__BIT__2__A:					// 0x57

            return {};

        case Z80__CB__BIT__3__B:					// 0x58
            return {};

        case Z80__CB__BIT__3__C:					// 0x59
            return {};

        case Z80__CB__BIT__3__D:					// 0x5a
            return {};

        case Z80__CB__BIT__3__E:					// 0x5b
            return {};

        case Z80__CB__BIT__3__H:					// 0x5c
            return {};

        case Z80__CB__BIT__3__L:					// 0x5d
            return {};

        case Z80__CB__BIT__3__INDIRECT_HL:	// 0x5e
            return {};

        case Z80__CB__BIT__3__A:					// 0x5f

            return {};

        case Z80__CB__BIT__4__B:					// 0x60
            return {};

        case Z80__CB__BIT__4__C:					// 0x61
            return {};

        case Z80__CB__BIT__4__D:					// 0x62
            return {};

        case Z80__CB__BIT__4__E:					// 0x63
            return {};

        case Z80__CB__BIT__4__H:					// 0x64
            return {};

        case Z80__CB__BIT__4__L:					// 0x65
            return {};

        case Z80__CB__BIT__4__INDIRECT_HL:	// 0x66
            return {};

        case Z80__CB__BIT__4__A:					// 0x67

            return {};

        case Z80__CB__BIT__5__B:					// 0x68
            return {};

        case Z80__CB__BIT__5__C:					// 0x69
            return {};

        case Z80__CB__BIT__5__D:					// 0x6a
            return {};

        case Z80__CB__BIT__5__E:					// 0x6b
            return {};

        case Z80__CB__BIT__5__H:					// 0x6c
            return {};

        case Z80__CB__BIT__5__L:					// 0x6d
            return {};

        case Z80__CB__BIT__5__INDIRECT_HL:	// 0x6e
            return {};

        case Z80__CB__BIT__5__A:					// 0x6f

            return {};

        case Z80__CB__BIT__6__B:					// 0x70
            return {};

        case Z80__CB__BIT__6__C:					// 0x71
            return {};

        case Z80__CB__BIT__6__D:					// 0x72
            return {};

        case Z80__CB__BIT__6__E:					// 0x73
            return {};

        case Z80__CB__BIT__6__H:					// 0x74
            return {};

        case Z80__CB__BIT__6__L:					// 0x75
            return {};

        case Z80__CB__BIT__6__INDIRECT_HL:	// 0x76
            return {};

        case Z80__CB__BIT__6__A:					// 0x77

            return {};

        case Z80__CB__BIT__7__B:					// 0x78
            return {};

        case Z80__CB__BIT__7__C:					// 0x79
            return {};

        case Z80__CB__BIT__7__D:					// 0x7a
            return {};

        case Z80__CB__BIT__7__E:					// 0x7b
            return {};

        case Z80__CB__BIT__7__H:					// 0x7c
            return {};

        case Z80__CB__BIT__7__L:					// 0x7d
            return {};

        case Z80__CB__BIT__7__INDIRECT_HL:	// 0x7e
            return {};

        case Z80__CB__BIT__7__A:					// 0x7f

            return {};

        case Z80__CB__RES__0__B:					// 0x80
            return {};

        case Z80__CB__RES__0__C:					// 0x81
            return {};

        case Z80__CB__RES__0__D:					// 0x82
            return {};

        case Z80__CB__RES__0__E:					// 0x83
            return {};

        case Z80__CB__RES__0__H:					// 0x84
            return {};

        case Z80__CB__RES__0__L:					// 0x85
            return {};

        case Z80__CB__RES__0__INDIRECT_HL:	// 0x86
            return {};

        case Z80__CB__RES__0__A:					// 0x87

            return {};

        case Z80__CB__RES__1__B:					// 0x88
            return {};

        case Z80__CB__RES__1__C:					// 0x89
            return {};

        case Z80__CB__RES__1__D:					// 0x8a
            return {};

        case Z80__CB__RES__1__E:					// 0x8b
            return {};

        case Z80__CB__RES__1__H:					// 0x8c
            return {};

        case Z80__CB__RES__1__L:					// 0x8d
            return {};

        case Z80__CB__RES__1__INDIRECT_HL:	// 0x8e
            return {};

        case Z80__CB__RES__1__A:					// 0x8f

            return {};

        case Z80__CB__RES__2__B:					// 0x90
            return {};

        case Z80__CB__RES__2__C:					// 0x91
            return {};

        case Z80__CB__RES__2__D:					// 0x92
            return {};

        case Z80__CB__RES__2__E:					// 0x93
            return {};

        case Z80__CB__RES__2__H:					// 0x94
            return {};

        case Z80__CB__RES__2__L:					// 0x95
            return {};

        case Z80__CB__RES__2__INDIRECT_HL:	// 0x96
            return {};

        case Z80__CB__RES__2__A:					// 0x97

            return {};

        case Z80__CB__RES__3__B:					// 0x98
            return {};

        case Z80__CB__RES__3__C:					// 0x99
            return {};

        case Z80__CB__RES__3__D:					// 0x9a
            return {};

        case Z80__CB__RES__3__E:					// 0x9b
            return {};

        case Z80__CB__RES__3__H:					// 0x9c
            return {};

        case Z80__CB__RES__3__L:					// 0x9d
            return {};

        case Z80__CB__RES__3__INDIRECT_HL:	// 0x9e
            return {};

        case Z80__CB__RES__3__A:					// 0x9f

            return {};

        case Z80__CB__RES__4__B:					// 0xa0
            return {};

        case Z80__CB__RES__4__C:					// 0xa1
            return {};

        case Z80__CB__RES__4__D:					// 0xa2
            return {};

        case Z80__CB__RES__4__E:					// 0xa3
            return {};

        case Z80__CB__RES__4__H:					// 0xa4
            return {};

        case Z80__CB__RES__4__L:					// 0xa5
            return {};

        case Z80__CB__RES__4__INDIRECT_HL:	// 0xa6
            return {};

        case Z80__CB__RES__4__A:					// 0xa7

            return {};

        case Z80__CB__RES__5__B:					// 0xa8
            return {};

        case Z80__CB__RES__5__C:					// 0xa9
            return {};

        case Z80__CB__RES__5__D:					// 0xaa
            return {};

        case Z80__CB__RES__5__E:					// 0xab
            return {};

        case Z80__CB__RES__5__H:					// 0xac
            return {};

        case Z80__CB__RES__5__L:					// 0xad
            return {};

        case Z80__CB__RES__5__INDIRECT_HL:	// 0xae
            return {};

        case Z80__CB__RES__5__A:					// 0xaf

            return {};

        case Z80__CB__RES__6__B:					// 0xb0
            return {};

        case Z80__CB__RES__6__C:					// 0xb1
            return {};

        case Z80__CB__RES__6__D:					// 0xb2
            return {};

        case Z80__CB__RES__6__E:					// 0xb3
            return {};

        case Z80__CB__RES__6__H:					// 0xb4
            return {};

        case Z80__CB__RES__6__L:					// 0xb5
            return {};

        case Z80__CB__RES__6__INDIRECT_HL:	// 0xb6
            return {};

        case Z80__CB__RES__6__A:					// 0xb7

            return {};

        case Z80__CB__RES__7__B:					// 0xb8
            return {};

        case Z80__CB__RES__7__C:					// 0xb9
            return {};

        case Z80__CB__RES__7__D:					// 0xba
            return {};

        case Z80__CB__RES__7__E:					// 0xbb
            return {};

        case Z80__CB__RES__7__H:					// 0xbc
            return {};

        case Z80__CB__RES__7__L:					// 0xbd
            return {};

        case Z80__CB__RES__7__INDIRECT_HL:	// 0xbe
            return {};

        case Z80__CB__RES__7__A:					// 0xbf

            return {};

        case Z80__CB__SET__0__B:					// 0xc0
            return {};

        case Z80__CB__SET__0__C:					// 0xc1
            return {};

        case Z80__CB__SET__0__D:					// 0xc2
            return {};

        case Z80__CB__SET__0__E:					// 0xc3
            return {};

        case Z80__CB__SET__0__H:					// 0xc4
            return {};

        case Z80__CB__SET__0__L:					// 0xc5
            return {};

        case Z80__CB__SET__0__INDIRECT_HL:	// 0xc6
            return {};

        case Z80__CB__SET__0__A:					// 0xc7

            return {};

        case Z80__CB__SET__1__B:					// 0xc8
            return {};

        case Z80__CB__SET__1__C:					// 0xc9
            return {};

        case Z80__CB__SET__1__D:					// 0xca
            return {};

        case Z80__CB__SET__1__E:					// 0xcb
            return {};

        case Z80__CB__SET__1__H:					// 0xcc
            return {};

        case Z80__CB__SET__1__L:					// 0xcd
            return {};

        case Z80__CB__SET__1__INDIRECT_HL:	// 0xce
            return {};

        case Z80__CB__SET__1__A:					// 0xcf

            return {};

        case Z80__CB__SET__2__B:					// 0xd0
            return {};

        case Z80__CB__SET__2__C:					// 0xd1
            return {};

        case Z80__CB__SET__2__D:					// 0xd2
            return {};

        case Z80__CB__SET__2__E:					// 0xd3
            return {};

        case Z80__CB__SET__2__H:					// 0xd4
            return {};

        case Z80__CB__SET__2__L:					// 0xd5
            return {};

        case Z80__CB__SET__2__INDIRECT_HL:	// 0xd6
            return {};

        case Z80__CB__SET__2__A:					// 0xd7

            return {};

        case Z80__CB__SET__3__B:					// 0xd8
            return {};

        case Z80__CB__SET__3__C:					// 0xd9
            return {};

        case Z80__CB__SET__3__D:					// 0xda
            return {};

        case Z80__CB__SET__3__E:					// 0xdb
            return {};

        case Z80__CB__SET__3__H:					// 0xdc
            return {};

        case Z80__CB__SET__3__L:					// 0xdd
            return {};

        case Z80__CB__SET__3__INDIRECT_HL:	// 0xde
            return {};

        case Z80__CB__SET__3__A:					// 0xdf

            return {};

        case Z80__CB__SET__4__B:					// 0xe0
            return {};

        case Z80__CB__SET__4__C:					// 0xe1
            return {};

        case Z80__CB__SET__4__D:					// 0xe2
            return {};

        case Z80__CB__SET__4__E:					// 0xe3
            return {};

        case Z80__CB__SET__4__H:					// 0xe4
            return {};

        case Z80__CB__SET__4__L:					// 0xe5
            return {};

        case Z80__CB__SET__4__INDIRECT_HL:	// 0xe6
            return {};

        case Z80__CB__SET__4__A:					// 0xe7

            return {};

        case Z80__CB__SET__5__B:					// 0xe8
            return {};

        case Z80__CB__SET__5__C:					// 0xe9
            return {};

        case Z80__CB__SET__5__D:					// 0xea
            return {};

        case Z80__CB__SET__5__E:					// 0xeb
            return {};

        case Z80__CB__SET__5__H:					// 0xec
            return {};

        case Z80__CB__SET__5__L:					// 0xed
            return {};

        case Z80__CB__SET__5__INDIRECT_HL:	// 0xee
            return {};

        case Z80__CB__SET__5__A:					// 0xef

            return {};

        case Z80__CB__SET__6__B:					// 0xf0
            return {};

        case Z80__CB__SET__6__C:					// 0xf1
            return {};

        case Z80__CB__SET__6__D:					// 0xf2
            return {};

        case Z80__CB__SET__6__E:					// 0xf3
            return {};

        case Z80__CB__SET__6__H:					// 0xf4
            return {};

        case Z80__CB__SET__6__L:					// 0xf5
            return {};

        case Z80__CB__SET__6__INDIRECT_HL:	// 0xf6
            return {};

        case Z80__CB__SET__6__A:					// 0xf7

            return {};

        case Z80__CB__SET__7__B:					// 0xf8
            return {};

        case Z80__CB__SET__7__C:					// 0xf9
            return {};

        case Z80__CB__SET__7__D:					// 0xfa
            return {};

        case Z80__CB__SET__7__E:					// 0xfb
            return {};

        case Z80__CB__SET__7__H:					// 0xfc
            return {};

        case Z80__CB__SET__7__L:					// 0xfd
            return {};

        case Z80__CB__SET__7__INDIRECT_HL:	// 0xfe
            return {};

        case Z80__CB__SET__7__A:					// 0xff
            return {};
    }
    
    std::cerr << "disassembly of opcode 0xcb 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}

Mnemonic Disassembler::disassembleOneEd(const Z80::UnsignedByte * machineCode)
{

    std::cerr << "disassembly of opcode 0xed 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}

Mnemonic Disassembler::disassembleOneDdOrFd(Register16 reg, const Z80::UnsignedByte * machineCode)
{

    std::cerr << "disassembly of opcode 0xed 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}
