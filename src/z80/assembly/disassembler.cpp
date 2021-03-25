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

    /**
     * Helper to fetch enough bytes from the memory to disassemble a single instruction at a given address.
     *
     * @param memory
     * @param address
     * @param machineCode
     */
    void fetchInstructionMachineCode(::Z80::Z80::MemoryType * memory, int address, UnsignedByte machineCode[4])
    {
        auto bytesAvailable = memory->size() - address;

        if (bytesAvailable < 4) {
            memory->readBytes(address, bytesAvailable, machineCode);
            memory->readBytes(0, 4 - bytesAvailable, machineCode + bytesAvailable);
        } else {
            memory->readBytes(address, 4, machineCode);
        }
    }
}

Disassembler::Mnemonics Disassembler::disassembleFrom(int address, int maxCount) const
{
    static UnsignedByte machineCode[4];
    Mnemonics ret;

    while (0 != maxCount && address < m_memory->size()) {
        fetchInstructionMachineCode(m_memory, address, machineCode);
        auto mnemonic = disassembleOne(machineCode);
        address += mnemonic.size;
        ret.push_back(std::move(mnemonic));

        if (0 < maxCount) {
            --maxCount;
        }
    }

    return ret;
}

Mnemonic Disassembler::nextMnemonic()
{
    static UnsignedByte machineCode[4];

    if (m_pc >= m_memory->size()) {
        return {
            Instruction::NOP,
            {},
            1,
        };
    }

    fetchInstructionMachineCode(m_memory, m_pc, machineCode);
    auto mnemonic = disassembleOne(machineCode);
    m_pc += mnemonic.size;
    return mnemonic;
}

Mnemonic Disassembler::disassembleOne(const ::Z80::UnsignedByte * machineCode)
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

Mnemonic Disassembler::disassembleOnePlain(const ::Z80::UnsignedByte * machineCode)
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
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::BC,},
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
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::BC,},
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
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::DE,},
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
                            { .mode = AddressingMode::Register16Indirect, .register16 =Register16::DE,},
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
                            { .mode = AddressingMode::Register16Indirect, .register16 =Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__DEC__INDIRECT_HL:                // 0x35
            return {
                    Instruction::DEC,
                    {
                            { .mode = AddressingMode::Register16Indirect, .register16 =Register16::HL,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__N:                // 0x36
            return {
                Instruction::LD,
                {
                    { .mode = AddressingMode::Register16Indirect, .register16 =Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__C:                // 0x71
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__D:                // 0x72
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__E:                // 0x73
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__H:                // 0x74
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    1,
            };

        case Z80__PLAIN__LD__INDIRECT_HL__L:                // 0x75
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::SP,},
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
                    {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
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

Mnemonic Disassembler::disassembleOneCb(const ::Z80::UnsignedByte * machineCode)
{
    // NOTE all 0xcb prefix opcodes are this size
    static constexpr const UnsignedByte OpcodeSize = 2;

    switch (*machineCode) {
        case Z80__CB__RLC__B:					// 0x00
            return {
                Instruction::RLC,
                {
                        {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__CB__RLC__C:					// 0x01
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RLC__D:					// 0x02
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RLC__E:					// 0x03
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RLC__H:					// 0x04
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RLC__L:					// 0x05
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RLC__INDIRECT_HL:	// 0x06
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RLC__A:					// 0x07
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__B:					// 0x08
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__C:					// 0x09
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__D:					// 0x0a
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__E:					// 0x0b
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__H:					// 0x0c
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__L:					// 0x0d
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__INDIRECT_HL:	// 0x0e
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RRC__A:					// 0x0f
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__B:					// 0x10
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__C:					// 0x11
            return {
                Instruction::RL,
                {
                        {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                },
                OpcodeSize,
            };

        case Z80__CB__RL__D:					// 0x12
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__E:					// 0x13
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__H:					// 0x14
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__L:					// 0x15
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__INDIRECT_HL:		// 0x16
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RL__A:					// 0x17
            return {
                    Instruction::RL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__B:					// 0x18
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__C:					// 0x19
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__D:					// 0x1a
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__E:					// 0x1b
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__H:					// 0x1c
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__L:					// 0x1d
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__INDIRECT_HL:		// 0x1e
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RR__A:					// 0x1f
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__B:					// 0x20
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__C:					// 0x21
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__D:					// 0x22
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__E:					// 0x23
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__H:					// 0x24
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__L:					// 0x25
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__INDIRECT_HL:	// 0x26
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLA__A:					// 0x27
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__B:					// 0x28
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__C:					// 0x29
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__D:					// 0x2a
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__E:					// 0x2b
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__H:					// 0x2c
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__L:					// 0x2d
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__INDIRECT_HL:	// 0x2e
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRA__A:					// 0x2f
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__B:					// 0x30
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__C:					// 0x31
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__D:					// 0x32
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__E:					// 0x33
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__H:					// 0x34
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__L:					// 0x35
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__INDIRECT_HL:	// 0x36
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SLL__A:					// 0x37
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__B:					// 0x38
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__C:					// 0x39
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__D:					// 0x3a
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__E:					// 0x3b
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__H:					// 0x3c
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__L:					// 0x3d
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__INDIRECT_HL:	// 0x3e
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SRL__A:					// 0x3f
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__B:					// 0x40
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__C:					// 0x41
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__D:					// 0x42
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__E:					// 0x43
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__H:					// 0x44
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__L:					// 0x45
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__INDIRECT_HL:	// 0x46
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__0__A:					// 0x47
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__B:					// 0x48
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__C:					// 0x49
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__D:					// 0x4a
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__E:					// 0x4b
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__H:					// 0x4c
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__L:					// 0x4d
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__INDIRECT_HL:	// 0x4e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__1__A:					// 0x4f
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__B:					// 0x50
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__C:					// 0x51
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__D:					// 0x52
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__E:					// 0x53
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__H:					// 0x54
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__L:					// 0x55
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__INDIRECT_HL:	// 0x56
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__2__A:					// 0x57
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__B:					// 0x58
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__C:					// 0x59
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__D:					// 0x5a
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__E:					// 0x5b
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__H:					// 0x5c
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__L:					// 0x5d
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__INDIRECT_HL:	// 0x5e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__3__A:					// 0x5f
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__B:					// 0x60
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__C:					// 0x61
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__D:					// 0x62
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__E:					// 0x63
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__H:					// 0x64
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__L:					// 0x65
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__INDIRECT_HL:	// 0x66
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__4__A:					// 0x67
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__B:					// 0x68
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__C:					// 0x69
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__D:					// 0x6a
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__E:					// 0x6b
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__H:					// 0x6c
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__L:					// 0x6d
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__INDIRECT_HL:	// 0x6e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__5__A:					// 0x6f
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__B:					// 0x70
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__C:					// 0x71
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__D:					// 0x72
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__E:					// 0x73
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__H:					// 0x74
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__L:					// 0x75
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__INDIRECT_HL:	// 0x76
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__6__A:					// 0x77
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__B:					// 0x78
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__C:					// 0x79
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__D:					// 0x7a
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__E:					// 0x7b
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__H:					// 0x7c
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__L:					// 0x7d
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__INDIRECT_HL:	// 0x7e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__BIT__7__A:					// 0x7f
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__B:					// 0x80
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__C:					// 0x81
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__D:					// 0x82
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__E:					// 0x83
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__H:					// 0x84
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__L:					// 0x85
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__INDIRECT_HL:	// 0x86
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__0__A:					// 0x87
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__B:					// 0x88
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__C:					// 0x89
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__D:					// 0x8a
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__E:					// 0x8b
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__H:					// 0x8c
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__L:					// 0x8d
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__INDIRECT_HL:	// 0x8e
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__1__A:					// 0x8f
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__B:					// 0x90
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__C:					// 0x91
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__D:					// 0x92
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__E:					// 0x93
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__H:					// 0x94
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__L:					// 0x95
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__INDIRECT_HL:	// 0x96
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__2__A:					// 0x97
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__B:					// 0x98
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__C:					// 0x99
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__D:					// 0x9a
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__E:					// 0x9b
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__H:					// 0x9c
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__L:					// 0x9d
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__INDIRECT_HL:	// 0x9e
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__3__A:					// 0x9f
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__B:					// 0xa0
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__C:					// 0xa1
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__D:					// 0xa2
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__E:					// 0xa3
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__H:					// 0xa4
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__L:					// 0xa5
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__INDIRECT_HL:	// 0xa6
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__4__A:					// 0xa7
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__B:					// 0xa8
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__C:					// 0xa9
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__D:					// 0xaa
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__E:					// 0xab
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__H:					// 0xac
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__L:					// 0xad
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__INDIRECT_HL:	// 0xae
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__5__A:					// 0xaf
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__B:					// 0xb0
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__C:					// 0xb1
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__D:					// 0xb2
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__E:					// 0xb3
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__H:					// 0xb4
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__L:					// 0xb5
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__INDIRECT_HL:	// 0xb6
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__6__A:					// 0xb7
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__B:					// 0xb8
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__C:					// 0xb9
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__D:					// 0xba
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__E:					// 0xbb
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__H:					// 0xbc
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__L:					// 0xbd
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__INDIRECT_HL:	// 0xbe
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__RES__7__A:					// 0xbf
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__B:					// 0xc0
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__C:					// 0xc1
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__D:					// 0xc2
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__E:					// 0xc3
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__H:					// 0xc4
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__L:					// 0xc5
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__INDIRECT_HL:	// 0xc6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__0__A:					// 0xc7
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__B:					// 0xc8
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__C:					// 0xc9
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__D:					// 0xca
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__E:					// 0xcb
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__H:					// 0xcc
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__L:					// 0xcd
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__INDIRECT_HL:	// 0xce
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__1__A:					// 0xcf
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__B:					// 0xd0
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__C:					// 0xd1
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__D:					// 0xd2
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__E:					// 0xd3
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__H:					// 0xd4
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__L:					// 0xd5
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__INDIRECT_HL:	// 0xd6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__2__A:					// 0xd7
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__B:					// 0xd8
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__C:					// 0xd9
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__D:					// 0xda
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__E:					// 0xdb
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__H:					// 0xdc
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__L:					// 0xdd
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__INDIRECT_HL:	// 0xde
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__3__A:					// 0xdf
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__B:					// 0xe0
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__CB__SET__4__C:					// 0xe1
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__D:					// 0xe2
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__E:					// 0xe3
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__H:					// 0xe4
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__L:					// 0xe5
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__INDIRECT_HL:	// 0xe6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__4__A:					// 0xe7
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__B:					// 0xe8
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__C:					// 0xe9
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__D:					// 0xea
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__E:					// 0xeb
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__H:					// 0xec
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__L:					// 0xed
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__INDIRECT_HL:	// 0xee
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__5__A:					// 0xef
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__B:					// 0xf0
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__CB__SET__6__C:					// 0xf1
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__D:					// 0xf2
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__E:					// 0xf3
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__H:					// 0xf4
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__L:					// 0xf5
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__INDIRECT_HL:	// 0xf6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__6__A:					// 0xf7
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__B:					// 0xf8
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__C:					// 0xf9
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__D:					// 0xfa
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__E:					// 0xfb
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__H:					// 0xfc
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__L:					// 0xfd
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__INDIRECT_HL:	// 0xfe
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    OpcodeSize,
            };

        case Z80__CB__SET__7__A:					// 0xff
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    OpcodeSize,
            };
    }
    
    std::cerr << "disassembly of opcode 0xcb 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}

Mnemonic Disassembler::disassembleOneEd(const ::Z80::UnsignedByte * machineCode)
{
    switch (*machineCode)
    {
        case Z80__ED__IN__B__INDIRECT_C:            // 0x40
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__B:            // 0x41
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B},
                    },
                    2,
            };

        case Z80__ED__SBC__HL__BC:                    // 0x42
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                    },
                    2,
            };

        case Z80__ED__LD__INDIRECT_NN__BC:        // 0x43
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                    },
                    4,
            };

        case Z80__ED__NEG:                                // 0x44
            return {
                Instruction::NEG,
                {},
                2,
            };

        case Z80__ED__RETN:                            // 0x45
            return {
                    Instruction::RETN,
                    {},
                    2,
            };

        case Z80__ED__IM__0:                            // 0x46
            return {
                    Instruction::IM0,
                    {},
                    2,
            };

        case Z80__ED__LD__I__A:                        // 0x47
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::I},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                    },
                    2,
            };

        case Z80__ED__IN__C__INDIRECT_C:            // 0x48
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__C:            // 0x49
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__ADC__HL__BC:                    // 0x4a
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                    },
                    2,
            };

        case Z80__ED__LD__BC__INDIRECT_NN:        // 0x4b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X4C:                // 0x4c
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETI:                            // 0x4d
            return {
                    Instruction::RETI,
                    {},
                    2,
            };

        case Z80__ED__IM__0__0XED__0X4E:            // 0x4e
            return {
                    Instruction::IM0,
                    {},
                    2,
            };

        case Z80__ED__LD__R__A:                        // 0x4f
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::R},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                    },
                    2,
            };

        case Z80__ED__IN__D__INDIRECT_C:            // 0x50
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__D:            // 0x51
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D},
                    },
                    2,
            };

        case Z80__ED__SBC__HL__DE:                    // 0x52
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                    },
                    2,
            };

        case Z80__ED__LD__INDIRECT_NN__DE:        // 0x53
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X54:                // 0x54
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETN__0XED__0X55:            // 0x55
            return {
                    Instruction::RETN,
                    {},
                    2,
            };

        case Z80__ED__IM__1:                            // 0x56
            return {
                    Instruction::IM1,
                    {},
                    2,
            };

        case Z80__ED__LD__A__I:                        // 0x57
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                            {.mode = AddressingMode::Register8, .register8 = Register8::I},
                    },
                    4,
            };

        case Z80__ED__IN__E__INDIRECT_C:            // 0x58
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__E:            // 0x59
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E},
                    },
                    2,
            };

        case Z80__ED__ADC__HL__DE:                    // 0x5a
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                    },
                    2,
            };

        case Z80__ED__LD__DE__INDIRECT_NN:        // 0x5b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X5C:                // 0x5c
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETI__0XED__0X5D:            // 0x5d
            return {
                    Instruction::RETI,
                    {},
                    2,
            };

        case Z80__ED__IM__2:                            // 0x5e
            return {
                    Instruction::IM2,
                    {},
                    2,
            };

        case Z80__ED__LD__A__R:                        // 0x5f
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                            {.mode = AddressingMode::Register8, .register8 = Register8::R},
                    },
                    2,
            };

        case Z80__ED__IN__H__INDIRECT_C:            // 0x60
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__H:            // 0x61
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H},
                    },
                    2,
            };

        case Z80__ED__SBC__HL__HL:                    // 0x62
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                    },
                    2,
            };

        case Z80__ED__LD__INDIRECT_NN__HL:        // 0x63
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X64:                // 0x64
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETN__0XED__0X65:            // 0x65
            return {
                    Instruction::RETN,
                    {},
                    2,
            };

        case Z80__ED__IM__0__0XED__0X66:            // 0x66
            return {
                    Instruction::IM0,
                    {},
                    2,
            };

        case Z80__ED__RRD:                                // 0x67
            return {
                    Instruction::RRD,
                    {},
                    2,
            };

        case Z80__ED__IN__L__INDIRECT_C:            // 0x68
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__L:            // 0x69
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L},
                    },
                    2,
            };

        case Z80__ED__ADC__HL__HL:                    // 0x6a
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                    },
                    2,
            };

        case Z80__ED__LD__HL__INDIRECT_NN:        // 0x6b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X6C:                // 0x6c
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETI__0XED__0X6D:            // 0x6d
            return {
                    Instruction::RETI,
                    {},
                    2,
            };

        case Z80__ED__IM__0__0XED__0X6E:            // 0x6e
            return {
                    Instruction::IM0,
                    {},
                    2,
            };

        case Z80__ED__RLD:                                // 0x6f
            return {
                    Instruction::RLD,
                    {},
                    2,
            };

        case Z80__ED__IN__INDIRECT_C:                // 0x70
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__0:            // 0x71
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Immediate, .unsignedByte = 0x00},
                    },
                    2,
            };

        case Z80__ED__SBC__HL__SP:                    // 0x72
            return {
                    Instruction::SBC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                    },
                    2,
            };

        case Z80__ED__LD__INDIRECT_NN__SP:        // 0x73
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X74:                // 0x74
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETN__0XED__0X75:            // 0x75
            return {
                    Instruction::RETN,
                    {},
                    2,
            };

        case Z80__ED__IM__1__0XED__0X76:            // 0x76
            return {
                    Instruction::IM1,
                    {},
                    2,
            };

        case Z80__ED__IN__A__INDIRECT_C:            // 0x78
            return {
                    Instruction::IN,
                    {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    2,
            };

        case Z80__ED__OUT__INDIRECT_C__A:            // 0x79
            return {
                    Instruction::OUT,
                    {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                    },
                    2,
            };

        case Z80__ED__ADC__HL__SP:                    // 0x7a
            return {
                    Instruction::ADC,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                    },
                    2,
            };

        case Z80__ED__LD__SP__INDIRECT_NN:        // 0x7b
            return {
                    Instruction::LD,
                    {
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    4,
            };

        case Z80__ED__NEG__0XED__0X7C:                // 0x7c
            return {
                    Instruction::NEG,
                    {},
                    2,
            };

        case Z80__ED__RETI__0XED__0X7D:            // 0x7d
            return {
                    Instruction::RETI,
                    {},
                    2,
            };

        case Z80__ED__IM__2__0XED__0X7E:            // 0x7e
            return {
                    Instruction::IM2,
                    {},
                    2,
            };

        case Z80__ED__LDI:                                // 0xa0
            return {
                    Instruction::LDI,
                    {},
                    2,
            };

        case Z80__ED__CPI:                                // 0xa1
            return {
                    Instruction::CPI,
                    {},
                    2,
            };

        case Z80__ED__INI:                                // 0xa2
            return {
                    Instruction::INI,
                    {},
                    2,
            };

        case Z80__ED__OUTI:                            // 0xa3
            return {
                    Instruction::OUTI,
                    {},
                    2,
            };

        case Z80__ED__LDD:                                // 0xa8
            return {
                    Instruction::LDD,
                    {},
                    2,
            };

        case Z80__ED__CPD:                                // 0xa9
            return {
                    Instruction::CPD,
                    {},
                    2,
            };

        case Z80__ED__IND:                                // 0xaa
            return {
                    Instruction::IND,
                    {},
                    2,
            };

        case Z80__ED__OUTD:                            // 0xab
            return {
                    Instruction::OUTD,
                    {},
                    2,
            };

        case Z80__ED__LDIR:                            // 0xb0
            return {
                    Instruction::LDIR,
                    {},
                    2,
            };

        case Z80__ED__CPIR:                            // 0xb1
            return {
                    Instruction::CPIR,
                    {},
                    2,
            };

        case Z80__ED__INIR:                            // 0xb2
            return {
                    Instruction::INIR,
                    {},
                    2,
            };

        case Z80__ED__OTIR:                            // 0xb3
            return {
                    Instruction::OTIR,
                    {},
                    2,
            };

        case Z80__ED__LDDR:                            // 0xb8
            return {
                    Instruction::LDDR,
                    {},
                    2,
            };

        case Z80__ED__CPDR:                            // 0xb9
            return {
                    Instruction::CPDR,
                    {},
                    2,
            };

        case Z80__ED__INDR:                            // 0xba
            return {
                    Instruction::INDR,
                    {},
                    2,
            };

        case Z80__ED__OTDR:                            // 0xbb
            return {
                    Instruction::OTDR,
                    {},
                    2,
            };

        case Z80__ED__NOP__0XED__0X00:                // 0x00
        case Z80__ED__NOP__0XED__0X01:                // 0x01
        case Z80__ED__NOP__0XED__0X02:                // 0x02
        case Z80__ED__NOP__0XED__0X03:                // 0x03
        case Z80__ED__NOP__0XED__0X04:                // 0x04
        case Z80__ED__NOP__0XED__0X05:                // 0x05
        case Z80__ED__NOP__0XED__0X06:                // 0x06
        case Z80__ED__NOP__0XED__0X07:                // 0x07
        case Z80__ED__NOP__0XED__0X08:                // 0x08
        case Z80__ED__NOP__0XED__0X09:                // 0x09
        case Z80__ED__NOP__0XED__0X0A:                // 0x0a
        case Z80__ED__NOP__0XED__0X0B:                // 0x0b
        case Z80__ED__NOP__0XED__0X0C:                // 0x0c
        case Z80__ED__NOP__0XED__0X0D:                // 0x0d
        case Z80__ED__NOP__0XED__0X0E:                // 0x0e
        case Z80__ED__NOP__0XED__0X0F:                // 0x0f
        case Z80__ED__NOP__0XED__0X10:                // 0x10
        case Z80__ED__NOP__0XED__0X11:                // 0x11
        case Z80__ED__NOP__0XED__0X12:                // 0x12
        case Z80__ED__NOP__0XED__0X13:                // 0x13
        case Z80__ED__NOP__0XED__0X14:                // 0x14
        case Z80__ED__NOP__0XED__0X15:                // 0x15
        case Z80__ED__NOP__0XED__0X16:                // 0x16
        case Z80__ED__NOP__0XED__0X17:                // 0x17
        case Z80__ED__NOP__0XED__0X18:                // 0x18
        case Z80__ED__NOP__0XED__0X19:                // 0x19
        case Z80__ED__NOP__0XED__0X1A:                // 0x1a
        case Z80__ED__NOP__0XED__0X1B:                // 0x1b
        case Z80__ED__NOP__0XED__0X1C:                // 0x1c
        case Z80__ED__NOP__0XED__0X1D:                // 0x1d
        case Z80__ED__NOP__0XED__0X1E:                // 0x1e
        case Z80__ED__NOP__0XED__0X1F:                // 0x1f
        case Z80__ED__NOP__0XED__0X20:                // 0x20
        case Z80__ED__NOP__0XED__0X21:                // 0x21
        case Z80__ED__NOP__0XED__0X22:                // 0x22
        case Z80__ED__NOP__0XED__0X23:                // 0x23
        case Z80__ED__NOP__0XED__0X24:                // 0x24
        case Z80__ED__NOP__0XED__0X25:                // 0x25
        case Z80__ED__NOP__0XED__0X26:                // 0x26
        case Z80__ED__NOP__0XED__0X27:                // 0x27
        case Z80__ED__NOP__0XED__0X28:                // 0x28
        case Z80__ED__NOP__0XED__0X29:                // 0x29
        case Z80__ED__NOP__0XED__0X2A:                // 0x2a
        case Z80__ED__NOP__0XED__0X2B:                // 0x2b
        case Z80__ED__NOP__0XED__0X2C:                // 0x2c
        case Z80__ED__NOP__0XED__0X2D:                // 0x2d
        case Z80__ED__NOP__0XED__0X2E:                // 0x2e
        case Z80__ED__NOP__0XED__0X2F:                // 0x2f
        case Z80__ED__NOP__0XED__0X30:                // 0x30
        case Z80__ED__NOP__0XED__0X31:                // 0x31
        case Z80__ED__NOP__0XED__0X32:                // 0x32
        case Z80__ED__NOP__0XED__0X33:                // 0x33
        case Z80__ED__NOP__0XED__0X34:                // 0x34
        case Z80__ED__NOP__0XED__0X35:                // 0x35
        case Z80__ED__NOP__0XED__0X36:                // 0x36
        case Z80__ED__NOP__0XED__0X37:                // 0x37
        case Z80__ED__NOP__0XED__0X38:                // 0x38
        case Z80__ED__NOP__0XED__0X39:                // 0x39
        case Z80__ED__NOP__0XED__0X3A:                // 0x3a
        case Z80__ED__NOP__0XED__0X3B:                // 0x3b
        case Z80__ED__NOP__0XED__0X3C:                // 0x3c
        case Z80__ED__NOP__0XED__0X3D:                // 0x3d
        case Z80__ED__NOP__0XED__0X3E:                // 0x3e
        case Z80__ED__NOP__0XED__0X3F:                // 0x3f

        case Z80__ED__NOP__0XED__0x77:                // 0x77
        case Z80__ED__NOP__0XED__0X7F:                // 0x7f
        case Z80__ED__NOP__0XED__0X80:                // 0x80
        case Z80__ED__NOP__0XED__0X81:                // 0x81
        case Z80__ED__NOP__0XED__0X82:                // 0x82
        case Z80__ED__NOP__0XED__0X83:                // 0x83
        case Z80__ED__NOP__0XED__0X84:                // 0x84
        case Z80__ED__NOP__0XED__0X85:                // 0x85
        case Z80__ED__NOP__0XED__0X86:                // 0x86
        case Z80__ED__NOP__0XED__0X87:                // 0x87
        case Z80__ED__NOP__0XED__0X88:                // 0x88
        case Z80__ED__NOP__0XED__0X89:                // 0x89
        case Z80__ED__NOP__0XED__0X8A:                // 0x8a
        case Z80__ED__NOP__0XED__0X8B:                // 0x8b
        case Z80__ED__NOP__0XED__0X8C:                // 0x8c
        case Z80__ED__NOP__0XED__0X8D:                // 0x8d
        case Z80__ED__NOP__0XED__0X8E:                // 0x8e
        case Z80__ED__NOP__0XED__0X8F:                // 0x8f
        case Z80__ED__NOP__0XED__0X90:                // 0x90
        case Z80__ED__NOP__0XED__0X91:                // 0x91
        case Z80__ED__NOP__0XED__0X92:                // 0x92
        case Z80__ED__NOP__0XED__0X93:                // 0x93
        case Z80__ED__NOP__0XED__0X94:                // 0x94
        case Z80__ED__NOP__0XED__0X95:                // 0x95
        case Z80__ED__NOP__0XED__0X96:                // 0x96
        case Z80__ED__NOP__0XED__0X97:                // 0x97
        case Z80__ED__NOP__0XED__0X98:                // 0x98
        case Z80__ED__NOP__0XED__0X99:                // 0x99
        case Z80__ED__NOP__0XED__0X9A:                // 0x9a
        case Z80__ED__NOP__0XED__0X9B:                // 0x9b
        case Z80__ED__NOP__0XED__0X9C:                // 0x9c
        case Z80__ED__NOP__0XED__0X9D:                // 0x9d
        case Z80__ED__NOP__0XED__0X9E:                // 0x9e
        case Z80__ED__NOP__0XED__0X9F:                // 0x9f

        case Z80__ED__NOP__0XED__0XA4:                // 0xa4
        case Z80__ED__NOP__0XED__0XA5:                // 0xa5
        case Z80__ED__NOP__0XED__0XA6:                // 0xa6
        case Z80__ED__NOP__0XED__0XA7:                // 0xa7
        
        case Z80__ED__NOP__0XED__0XAC:                // 0xac
        case Z80__ED__NOP__0XED__0XAD:                // 0xad
        case Z80__ED__NOP__0XED__0XAE:                // 0xae
        case Z80__ED__NOP__0XED__0XAF:                // 0xaf

        case Z80__ED__NOP__0XED__0XB4:                // 0xb4
        case Z80__ED__NOP__0XED__0XB5:                // 0xb5
        case Z80__ED__NOP__0XED__0XB6:                // 0xb6
        case Z80__ED__NOP__0XED__0XB7:                // 0xb7

        case Z80__ED__NOP__0XED__0XBC:                // 0xbc
        case Z80__ED__NOP__0XED__0XBD:                // 0xbd
        case Z80__ED__NOP__0XED__0XBE:                // 0xbe
        case Z80__ED__NOP__0XED__0XBF:                // 0xbf
        case Z80__ED__NOP__0XED__0XC0:                // 0xc0
        case Z80__ED__NOP__0XED__0XC1:                // 0xc1
        case Z80__ED__NOP__0XED__0XC2:                // 0xc2
        case Z80__ED__NOP__0XED__0XC3:                // 0xc3
        case Z80__ED__NOP__0XED__0XC4:                // 0xc4
        case Z80__ED__NOP__0XED__0XC5:                // 0xc5
        case Z80__ED__NOP__0XED__0XC6:                // 0xc6
        case Z80__ED__NOP__0XED__0XC7:                // 0xc7
        case Z80__ED__NOP__0XED__0XC8:                // 0xc8
        case Z80__ED__NOP__0XED__0XC9:                // 0xc9
        case Z80__ED__NOP__0XED__0XCA:                // 0xca
        case Z80__ED__NOP__0XED__0XCB:                // 0xcb
        case Z80__ED__NOP__0XED__0XCC:                // 0xcc
        case Z80__ED__NOP__0XED__0XCD:                // 0xcd
        case Z80__ED__NOP__0XED__0XCE:                // 0xce
        case Z80__ED__NOP__0XED__0XCF:                // 0xcf
        case Z80__ED__NOP__0XED__0XD0:                // 0xd0
        case Z80__ED__NOP__0XED__0XD1:                // 0xd1
        case Z80__ED__NOP__0XED__0XD2:                // 0xd2
        case Z80__ED__NOP__0XED__0XD3:                // 0xd3
        case Z80__ED__NOP__0XED__0XD4:                // 0xd4
        case Z80__ED__NOP__0XED__0XD5:                // 0xd5
        case Z80__ED__NOP__0XED__0XD6:                // 0xd6
        case Z80__ED__NOP__0XED__0XD7:                // 0xd7
        case Z80__ED__NOP__0XED__0XD8:                // 0xd8
        case Z80__ED__NOP__0XED__0XD9:                // 0xd9
        case Z80__ED__NOP__0XED__0XDA:                // 0xda
        case Z80__ED__NOP__0XED__0XDB:                // 0xdb
        case Z80__ED__NOP__0XED__0XDC:                // 0xdc
        case Z80__ED__NOP__0XED__0XDD:                // 0xdd
        case Z80__ED__NOP__0XED__0XDE:                // 0xde
        case Z80__ED__NOP__0XED__0XDF:                // 0xdf
        case Z80__ED__NOP__0XED__0XE0:                // 0xe0
        case Z80__ED__NOP__0XED__0XE1:                // 0xe1
        case Z80__ED__NOP__0XED__0XE2:                // 0xe2
        case Z80__ED__NOP__0XED__0XE3:                // 0xe3
        case Z80__ED__NOP__0XED__0XE4:                // 0xe4
        case Z80__ED__NOP__0XED__0XE5:                // 0xe5
        case Z80__ED__NOP__0XED__0XE6:                // 0xe6
        case Z80__ED__NOP__0XED__0XE7:                // 0xe7
        case Z80__ED__NOP__0XED__0XE8:                // 0xe8
        case Z80__ED__NOP__0XED__0XE9:                // 0xe9
        case Z80__ED__NOP__0XED__0XEA:                // 0xea
        case Z80__ED__NOP__0XED__0XEB:                // 0xeb
        case Z80__ED__NOP__0XED__0XEC:                // 0xec
        case Z80__ED__NOP__0XED__0XED:                // 0xed
        case Z80__ED__NOP__0XED__0XEE:                // 0xee
        case Z80__ED__NOP__0XED__0XEF:                // 0xef
        case Z80__ED__NOP__0XED__0XF0:                // 0xf0
        case Z80__ED__NOP__0XED__0XF1:                // 0xf1
        case Z80__ED__NOP__0XED__0XF2:                // 0xf2
        case Z80__ED__NOP__0XED__0XF3:                // 0xf3
        case Z80__ED__NOP__0XED__0XF4:                // 0xf4
        case Z80__ED__NOP__0XED__0XF5:                // 0xf5
        case Z80__ED__NOP__0XED__0XF6:                // 0xf6
        case Z80__ED__NOP__0XED__0XF7:                // 0xf7
        case Z80__ED__NOP__0XED__0XF8:                // 0xf8
        case Z80__ED__NOP__0XED__0XF9:                // 0xf9
        case Z80__ED__NOP__0XED__0XFA:                // 0xfa
        case Z80__ED__NOP__0XED__0XFB:                // 0xfb
        case Z80__ED__NOP__0XED__0XFC:                // 0xfc
        case Z80__ED__NOP__0XED__0XFD:                // 0xfd
        case Z80__ED__NOP__0XED__0XFE:                // 0xfe
        case Z80__ED__NOP__0XED__0XFF:                // 0xff
            return {
                    Instruction::NOP,
                    {},
                    2,
            };
    }

    std::cerr << "disassembly of opcode 0xed 0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}

Mnemonic Disassembler::disassembleOneDdOrFd(Register16 reg, const ::Z80::UnsignedByte * machineCode)
{
    switch (*machineCode) {
        case Z80__DD_OR_FD__INC__INDIRECT_IX_d_OR_IY_d:                // 0x34
            return {
                Instruction::INC,
                {
                    { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__DEC__INDIRECT_IX_d_OR_IY_d:                // 0x35
            return {
                Instruction::DEC,
                {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__N:                // 0x36
            return {
                Instruction::LD,
                {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                        { .mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1), },
                },
                4,
            };

        case Z80__DD_OR_FD__JR__C__d:                // 0x38
            return {
                Instruction::JRC,
                {
                    { .mode = AddressingMode::Relative, .unsignedByte = *(machineCode + 1), },
                },
                3,
            };

        case Z80__DD_OR_FD__LD__B__INDIRECT_IX_d_OR_IY_d:                // 0x46
            return {
                Instruction::LD,
                {
                        { .mode = AddressingMode::Register8, .register8 = Register8::B, },
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__LD__C__INDIRECT_IX_d_OR_IY_d:                // 0x4e
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::C, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__D__INDIRECT_IX_d_OR_IY_d:                // 0x56
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::D, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__E__INDIRECT_IX_d_OR_IY_d:                // 0x5e
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::E, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__H__INDIRECT_IX_d_OR_IY_d:                // 0x66
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::H, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__L__INDIRECT_IX_d_OR_IY_d:                // 0x6e
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::L, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__B:                // 0x70
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::B, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__C:                // 0x71
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::C, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__D:                // 0x72
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::D, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__E:                // 0x73
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::E, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__H:                // 0x74
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::H, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__L:                // 0x75
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::L, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__INDIRECT_IX_d_OR_IY_d__A:                // 0x77
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__LD__A__INDIRECT_IX_d_OR_IY_d:                // 0x7e
            return {
                    Instruction::LD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__ADD__A__INDIRECT_IX_d_OR_IY_d:                // 0x86
            return {
                Instruction::ADD,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__ADC__A__INDIRECT_IX_d_OR_IY_d:                // 0x8e
            return {
                    Instruction::ADC,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__SUB__INDIRECT_IX_d_OR_IY_d:                // 0x96
            return {
                    Instruction::SUB,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__SBC__A__INDIRECT_IX_d_OR_IY_d:                // 0x9e
            return {
                    Instruction::SBC,
                    {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    3,
            };

        case Z80__DD_OR_FD__AND__INDIRECT_IX_d_OR_IY_d:                // 0xa6
            return {
                Instruction::AND,
                {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__XOR__INDIRECT_IX_d_OR_IY_d:                // 0xae
            return {
                Instruction::XOR,
                {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__OR__INDIRECT_IX_d_OR_IY_d:                // 0xb6
            return {
                Instruction::OR,
                {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__CP__INDIRECT_IX_d_OR_IY_d:                // 0xbe
            return {
                Instruction::CP,
                {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                3,
            };

        case Z80__DD_OR_FD__PREFIX__CB:                // 0xcb
            return disassembleOneDdCbOrFdCb(reg, machineCode + 1);

        // these are all expensive replicas of the plain instructions
        case Z80__DD_OR_FD__NOP:                // 0x00
        case Z80__DD_OR_FD__LD__BC__NN:                // 0x01
        case Z80__DD_OR_FD__LD__INDIRECT_BC__A:                // 0x02
        case Z80__DD_OR_FD__INC__BC:                // 0x03
        case Z80__DD_OR_FD__INC__B:                // 0x04
        case Z80__DD_OR_FD__DEC__B:                // 0x05
        case Z80__DD_OR_FD__LD__B__N:                // 0x06
        case Z80__DD_OR_FD__RLCA:                // 0x07
        case Z80__DD_OR_FD__EX__AF__AF_SHADOW:                // 0x08
        case Z80__DD_OR_FD__ADD__IX_OR_IY__BC:                // 0x09
        case Z80__DD_OR_FD__LD__A__INDIRECT_BC:                // 0x0a
        case Z80__DD_OR_FD__DEC__BC:                // 0x0b
        case Z80__DD_OR_FD__INC__C:                // 0x0c
        case Z80__DD_OR_FD__DEC__C:                // 0x0d
        case Z80__DD_OR_FD__LD__C__N:                // 0x0e
        case Z80__DD_OR_FD__RRCA:                // 0x0f
        case Z80__DD_OR_FD__DJNZ__d:                // 0x10
        case Z80__DD_OR_FD__LD__DE__NN:                // 0x11
        case Z80__DD_OR_FD__LD__INDIRECT_DE__A:                // 0x12
        case Z80__DD_OR_FD__INC__DE:                // 0x13
        case Z80__DD_OR_FD__INC__D:                // 0x14
        case Z80__DD_OR_FD__DEC__D:                // 0x15
        case Z80__DD_OR_FD__LD__D__N:                // 0x16
        case Z80__DD_OR_FD__RLA:                // 0x17
        case Z80__DD_OR_FD__JR__d:                // 0x18
        case Z80__DD_OR_FD__ADD__IX_OR_IY__DE:                // 0x19
        case Z80__DD_OR_FD__LD__A__INDIRECT_DE:                // 0x1a
        case Z80__DD_OR_FD__DEC__DE:                // 0x1b
        case Z80__DD_OR_FD__INC__E:                // 0x1c
        case Z80__DD_OR_FD__DEC__E:                // 0x1d
        case Z80__DD_OR_FD__LD__E__N:                // 0x1e
        case Z80__DD_OR_FD__RRA:                // 0x1f
        case Z80__DD_OR_FD__JR__NZ__d:                // 0x20
        case Z80__DD_OR_FD__LD__IX_OR_IY__NN:                // 0x21
        case Z80__DD_OR_FD__LD__INDIRECT_NN__IX_OR_IY:                // 0x22
        case Z80__DD_OR_FD__INC__IX_OR_IY:                // 0x23
        case Z80__DD_OR_FD__INC__IXH_OR_IYH:                // 0x24
        case Z80__DD_OR_FD__DEC__IXH_OR_IYH:                // 0x25
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__N:                // 0x26
        case Z80__DD_OR_FD__DAA:                // 0x27
        case Z80__DD_OR_FD__JR__Z__d:                // 0x28
        case Z80__DD_OR_FD__ADD__IX_OR_IY__IX_OR_IY:                // 0x29
        case Z80__DD_OR_FD__LD__IX_OR_IY__INDIRECT_NN:                // 0x2a
        case Z80__DD_OR_FD__DEC__IX_OR_IY:                // 0x2b
        case Z80__DD_OR_FD__INC__IXL_OR_IYL:                // 0x2c
        case Z80__DD_OR_FD__DEC__IXL_OR_IYL:                // 0x2d
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__N:                // 0x2e
        case Z80__DD_OR_FD__CPL:                // 0x2f
        case Z80__DD_OR_FD__JR__NC__d:                // 0x30
        case Z80__DD_OR_FD__LD__SP__NN:                // 0x31
        case Z80__DD_OR_FD__LD__INDIRECT_NN__A:                // 0x32
        case Z80__DD_OR_FD__INC__SP:                // 0x33
        case Z80__DD_OR_FD__SCF:                // 0x37
        case Z80__DD_OR_FD__ADD__IX_OR_IY__SP:                // 0x39
        case Z80__DD_OR_FD__LD__A__INDIRECT_NN:                // 0x3a
        case Z80__DD_OR_FD__DEC__SP:                // 0x3b
        case Z80__DD_OR_FD__INC__A:                // 0x3c
        case Z80__DD_OR_FD__DEC__A:                // 0x3d
        case Z80__DD_OR_FD__LD__A__N:                // 0x3e
        case Z80__DD_OR_FD__CCF:                // 0x3f
        case Z80__DD_OR_FD__LD__B__B:                // 0x40
        case Z80__DD_OR_FD__LD__B__C:                // 0x41
        case Z80__DD_OR_FD__LD__B__D:                // 0x42
        case Z80__DD_OR_FD__LD__B__E:                // 0x43
        case Z80__DD_OR_FD__LD__B__IXH_OR_IYH:                // 0x44
        case Z80__DD_OR_FD__LD__B__IXL_OR_IYL:                // 0x45
        case Z80__DD_OR_FD__LD__B__A:                // 0x47
        case Z80__DD_OR_FD__LD__C__B:                // 0x48
        case Z80__DD_OR_FD__LD__C__C:                // 0x49
        case Z80__DD_OR_FD__LD__C__D:                // 0x4a
        case Z80__DD_OR_FD__LD__C__E:                // 0x4b
        case Z80__DD_OR_FD__LD__C__IXH_OR_IYH:                // 0x4c
        case Z80__DD_OR_FD__LD__C__IXL_OR_IYL:                // 0x4d
        case Z80__DD_OR_FD__LD__C__A:                // 0x4f
        case Z80__DD_OR_FD__LD__D__B:                // 0x50
        case Z80__DD_OR_FD__LD__D__C:                // 0x51
        case Z80__DD_OR_FD__LD__D__D:                // 0x52
        case Z80__DD_OR_FD__LD__D__E:                // 0x53
        case Z80__DD_OR_FD__LD__D__IXH_OR_IYH:                // 0x54
        case Z80__DD_OR_FD__LD__D__IXL_OR_IYL:                // 0x55
        case Z80__DD_OR_FD__LD__D__A:                // 0x57
        case Z80__DD_OR_FD__LD__E__B:                // 0x58
        case Z80__DD_OR_FD__LD__E__C:                // 0x59
        case Z80__DD_OR_FD__LD__E__D:                // 0x5a
        case Z80__DD_OR_FD__LD__E__E:                // 0x5b
        case Z80__DD_OR_FD__LD__E__IXH_OR_IYH:                // 0x5c
        case Z80__DD_OR_FD__LD__E__IXL_OR_IYL:                // 0x5d
        case Z80__DD_OR_FD__LD__E__A:                // 0x5f
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__B:                // 0x60
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__C:                // 0x61
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__D:                // 0x62
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__E:                // 0x63
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__IXH_OR_IYH:                // 0x64
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__IXL_OR_IYL:                // 0x65
        case Z80__DD_OR_FD__LD__IXH_OR_IYH__A:                // 0x67
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__B:                // 0x68
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__C:                // 0x69
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__D:                // 0x6a
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__E:                // 0x6b
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__IXH_OR_IYH:                // 0x6c
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__IXL_OR_IYL:                // 0x6d
        case Z80__DD_OR_FD__LD__IXL_OR_IYL__A:                // 0x6f
        case Z80__DD_OR_FD__HALT:                // 0x76
        case Z80__DD_OR_FD__LD__A__B:                // 0x78
        case Z80__DD_OR_FD__LD__A__C:                // 0x79
        case Z80__DD_OR_FD__LD__A__D:                // 0x7a
        case Z80__DD_OR_FD__LD__A__E:                // 0x7b
        case Z80__DD_OR_FD__LD__A__IXH_OR_IYH:                // 0x7c
        case Z80__DD_OR_FD__LD__A__IXL_OR_IYL:                // 0x7d
        case Z80__DD_OR_FD__LD__A__A:                // 0x7f
        case Z80__DD_OR_FD__ADD__A__B:                // 0x80
        case Z80__DD_OR_FD__ADD__A__C:                // 0x81
        case Z80__DD_OR_FD__ADD__A__D:                // 0x82
        case Z80__DD_OR_FD__ADD__A__E:                // 0x83
        case Z80__DD_OR_FD__ADD__A__IXH_OR_IYH:                // 0x84
        case Z80__DD_OR_FD__ADD__A__IXL_OR_IYL:                // 0x85
        case Z80__DD_OR_FD__ADD__A__A:                // 0x87
        case Z80__DD_OR_FD__ADC__A__B:                // 0x88
        case Z80__DD_OR_FD__ADC__A__C:                // 0x89
        case Z80__DD_OR_FD__ADC__A__D:                // 0x8a
        case Z80__DD_OR_FD__ADC__A__E:                // 0x8b
        case Z80__DD_OR_FD__ADC__A__IXH_OR_IYH:                // 0x8c
        case Z80__DD_OR_FD__ADC__A__IXL_OR_IYL:                // 0x8d
        case Z80__DD_OR_FD__ADC__A__A:                // 0x8f
        case Z80__DD_OR_FD__SUB__B:                // 0x90
        case Z80__DD_OR_FD__SUB__C:                // 0x91
        case Z80__DD_OR_FD__SUB__D:                // 0x92
        case Z80__DD_OR_FD__SUB__E:                // 0x93
        case Z80__DD_OR_FD__SUB__IXH_OR_IYH:                // 0x94
        case Z80__DD_OR_FD__SUB__IXL_OR_IYL:                // 0x95
        case Z80__DD_OR_FD__SUB__A:                // 0x97
        case Z80__DD_OR_FD__SBC__A__B:                // 0x98
        case Z80__DD_OR_FD__SBC__A__C:                // 0x99
        case Z80__DD_OR_FD__SBC__A__D:                // 0x9a
        case Z80__DD_OR_FD__SBC__A__E:                // 0x9b
        case Z80__DD_OR_FD__SBC__A__IXH_OR_IYH:                // 0x9c
        case Z80__DD_OR_FD__SBC__A__IXL_OR_IYL:                // 0x9d
        case Z80__DD_OR_FD__SBC__A__A:                // 0x9f
        case Z80__DD_OR_FD__AND__B:                // 0xa0
        case Z80__DD_OR_FD__AND__C:                // 0xa1
        case Z80__DD_OR_FD__AND__D:                // 0xa2
        case Z80__DD_OR_FD__AND__E:                // 0xa3
        case Z80__DD_OR_FD__AND__IXH_OR_IYH:                // 0xa4
        case Z80__DD_OR_FD__AND__IXL_OR_IYL:                // 0xa5
        case Z80__DD_OR_FD__AND__A:                // 0xa7
        case Z80__DD_OR_FD__XOR__B:                // 0xa8
        case Z80__DD_OR_FD__XOR__C:                // 0xa9
        case Z80__DD_OR_FD__XOR__D:                // 0xaa
        case Z80__DD_OR_FD__XOR__E:                // 0xab
        case Z80__DD_OR_FD__XOR__IXH_OR_IYH:                // 0xac
        case Z80__DD_OR_FD__XOR__IXL_OR_IYL:                // 0xad
        case Z80__DD_OR_FD__XOR__A:                // 0xaf
        case Z80__DD_OR_FD__OR__B:                // 0xb0
        case Z80__DD_OR_FD__OR__C:                // 0xb1
        case Z80__DD_OR_FD__OR__D:                // 0xb2
        case Z80__DD_OR_FD__OR__E:                // 0xb3
        case Z80__DD_OR_FD__OR__IXH_OR_IYH:                // 0xb4
        case Z80__DD_OR_FD__OR__IXL_OR_IYL:                // 0xb5
        case Z80__DD_OR_FD__OR__A:                // 0xb7
        case Z80__DD_OR_FD__CP__B:                // 0xb8
        case Z80__DD_OR_FD__CP__C:                // 0xb9
        case Z80__DD_OR_FD__CP__D:                // 0xba
        case Z80__DD_OR_FD__CP__E:                // 0xbb
        case Z80__DD_OR_FD__CP__IXH_OR_IYH:                // 0xbc
        case Z80__DD_OR_FD__CP__IXL_OR_IYL:                // 0xbd
        case Z80__DD_OR_FD__CP__A:                // 0xbf
        case Z80__DD_OR_FD__RET__NZ:                // 0xc0
        case Z80__DD_OR_FD__POP__BC:                // 0xc1
        case Z80__DD_OR_FD__JP__NZ__NN:                // 0xc2
        case Z80__DD_OR_FD__JP__NN:                // 0xc3
        case Z80__DD_OR_FD__CALL__NZ__NN:                // 0xc4
        case Z80__DD_OR_FD__PUSH__BC:                // 0xc5
        case Z80__DD_OR_FD__ADD__A__N:                // 0xc6
        case Z80__DD_OR_FD__RST__00:                // 0xc7
        case Z80__DD_OR_FD__RET__Z:                // 0xc8
        case Z80__DD_OR_FD__RET:                // 0xc9
        case Z80__DD_OR_FD__JP__Z__NN:                // 0xca
        case Z80__DD_OR_FD__CALL__Z__NN:                // 0xcc
        case Z80__DD_OR_FD__CALL__NN:                // 0xcd
        case Z80__DD_OR_FD__ADC__A__N:                // 0xce
        case Z80__DD_OR_FD__RST__08:                // 0xcf
        case Z80__DD_OR_FD__RET__NC:                // 0xd0
        case Z80__DD_OR_FD__POP__DE:                // 0xd1
        case Z80__DD_OR_FD__JP__NC__NN:                // 0xd2
        case Z80__DD_OR_FD__OUT__INDIRECT_N__A:                // 0xd3
        case Z80__DD_OR_FD__CALL__NC__NN:                // 0xd4
        case Z80__DD_OR_FD__PUSH__DE:                // 0xd5
        case Z80__DD_OR_FD__SUB__N:                // 0xd6
        case Z80__DD_OR_FD__RST__10:                // 0xd7
        case Z80__DD_OR_FD__RET__C:                // 0xd8
        case Z80__DD_OR_FD__EXX:                // 0xd9
        case Z80__DD_OR_FD__JP__C__NN:                // 0xda
        case Z80__DD_OR_FD__IN__A__INDIRECT_N:                // 0xdb
        case Z80__DD_OR_FD__CALL__C__NN:                // 0xdc
        case Z80__DD_OR_FD__SBC__A__N:                // 0xde
        case Z80__DD_OR_FD__RST__18:                // 0xdf
        case Z80__DD_OR_FD__RET__PO:                // 0xe0
        case Z80__DD_OR_FD__POP__IX_OR_IY:                // 0xe1
        case Z80__DD_OR_FD__JP__PO__NN:                // 0xe2
        case Z80__DD_OR_FD__EX__INDIRECT_SP__IX_OR_IY:                // 0xe3
        case Z80__DD_OR_FD__CALL__PO__NN:                // 0xe4
        case Z80__DD_OR_FD__PUSH__IX_OR_IY:                // 0xe5
        case Z80__DD_OR_FD__AND__N:                // 0xe6
        case Z80__DD_OR_FD__RST__20:                // 0xe7
        case Z80__DD_OR_FD__RET__PE:                // 0xe8
        case Z80__DD_OR_FD__JP__IX_OR_IY:                // 0xe9
        case Z80__DD_OR_FD__JP__PE__NN:                // 0xea
        case Z80__DD_OR_FD__EX__DE__HL:                // 0xeb
        case Z80__DD_OR_FD__CALL__PE__NN:                // 0xec
        case Z80__DD_OR_FD__PREFIX__ED:                // 0xed
        case Z80__DD_OR_FD__XOR__N:                // 0xee
        case Z80__DD_OR_FD__RST__28:                // 0xef
        case Z80__DD_OR_FD__RET__P:                // 0xf0
        case Z80__DD_OR_FD__POP__AF:                // 0xf1
        case Z80__DD_OR_FD__JP__P__NN:                // 0xf2
        case Z80__DD_OR_FD__DI:                // 0xf3
        case Z80__DD_OR_FD__CALL__P__NN:                // 0xf4
        case Z80__DD_OR_FD__PUSH__AF:                // 0xf5
        case Z80__DD_OR_FD__OR__N:                // 0xf6
        case Z80__DD_OR_FD__RST__30:                // 0xf7
        case Z80__DD_OR_FD__RET__M:                // 0xf8
        case Z80__DD_OR_FD__LD__SP__IX_OR_IY:                // 0xf9
        case Z80__DD_OR_FD__JP__M__NN:                // 0xfa
        case Z80__DD_OR_FD__EI:                // 0xfb
        case Z80__DD_OR_FD__CALL__M__NN:                // 0xfc
        case Z80__DD_OR_FD__CP__N:                // 0xfe
        case Z80__DD_OR_FD__RST__38:                // 0xff
        {
            auto mnemonic = disassembleOnePlain(machineCode);
            ++mnemonic.size;
            return mnemonic;
        }

        case Z80__DD_OR_FD__PREFIX__DD:                // 0xdd
        case Z80__DD_OR_FD__PREFIX__FD:                // 0xfd
        {
            // TODO this is not strictly correct - sequences of 0xdd/0xfd result in an IX/IY instruction based on the
            //  byte following the last 0xdd/0xfd in the sequence.
            return {
                Instruction::NOP,
                {},
                2,
            };
        }
    }

    std::cerr << "disassembly of opcode " << (Register16::IX == reg ? "0xdd " : "0xfd ") << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}

Mnemonic Disassembler::disassembleOneDdCbOrFdCb(Register16 reg, const ::Z80::UnsignedByte * machineCode)
{
    static constexpr const int OpcodeSize = 3;
    
    switch (*machineCode) {
        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__B:                       // 0x00
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__C:                       // 0x01
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__D:                       // 0x02
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__E:                       // 0x03
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__H:                       // 0x04
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__L:                       // 0x05
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d:                          // 0x06
            return {
                    Instruction::RLC,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RLC__INDIRECT_IX_d_OR_IY_d__A:                       // 0x07
            return {
                Instruction::RLC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__B:                       // 0x08
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__C:                       // 0x09
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__D:                       // 0x0a
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__E:                       // 0x0b
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__H:                       // 0x0c
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__L:                       // 0x0d
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d:                          // 0x0e
            return {
                    Instruction::RRC,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RRC__INDIRECT_IX_d_OR_IY_d__A:                       // 0x0f
            return {
                Instruction::RRC,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };


        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__B:                        // 0x10
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__C:                        // 0x11
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__D:                        // 0x12
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__E:                        // 0x13
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__H:                        // 0x14
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__L:                        // 0x15
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d:                           // 0x16
            return {
                Instruction::RL,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RL__INDIRECT_IX_d_OR_IY_d__A:                        // 0x17
            return {
                Instruction::RL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };


        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__B:                        // 0x18
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__C:                        // 0x19
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__D:                        // 0x1a
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__E:                        // 0x1b
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__H:                        // 0x1c
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__L:                        // 0x1d
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d:                          // 0x1e
            return {
                    Instruction::RR,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RR__INDIRECT_IX_d_OR_IY_d__A:                        // 0x1f
            return {
                Instruction::RR,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };


        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__B:                       // 0x20
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__C:                       // 0x21
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__D:                       // 0x22
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__E:                       // 0x23
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__H:                       // 0x24
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__L:                       // 0x25
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d:                          // 0x26
            return {
                    Instruction::SLA,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLA__INDIRECT_IX_d_OR_IY_d__A:                       // 0x27
            return {
                Instruction::SLA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };


        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__B:                       // 0x28
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__C:                       // 0x29
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__D:                       // 0x2a
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__E:                       // 0x2b
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__H:                       // 0x2c
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__L:                       // 0x2d
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d:                          // 0x2e
            return {
                    Instruction::SRA,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRA__INDIRECT_IX_d_OR_IY_d__A:                       // 0x2f
            return {
                Instruction::SRA,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };


        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__B:                       // 0x30
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__C:                       // 0x31
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__D:                       // 0x32
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__E:                       // 0x33
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__H:                       // 0x34
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__L:                       // 0x35
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d:                          // 0x36
            return {
                    Instruction::SLL,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SLL__INDIRECT_IX_d_OR_IY_d__A:                       // 0x37
            return {
                Instruction::SLL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };


        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__B:                       // 0x38
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__C:                       // 0x39
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__D:                       // 0x3a
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__E:                       // 0x3b
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__H:                       // 0x3c
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__L:                       // 0x3d
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d:                          // 0x3e
            return {
                    Instruction::SRL,
                    {
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SRL__INDIRECT_IX_d_OR_IY_d__A:                       // 0x3f
            return {
                Instruction::SRL,
                {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__B:                    // 0x40
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__C:                    // 0x41
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__D:                    // 0x42
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__E:                    // 0x43
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__H:                    // 0x44
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__L:                    // 0x45
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d:                          // 0x06
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__0__INDIRECT_IX_d_OR_IY_d__A:                    // 0x47
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__B:                    // 0x48
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__C:                    // 0x49
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__D:                    // 0x4a
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__E:                    // 0x4b
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__H:                    // 0x4c
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__L:                    // 0x4d
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d:                          // 0x4e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__1__INDIRECT_IX_d_OR_IY_d__A:                    // 0x4f
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__B:                    // 0x50
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__C:                    // 0x51
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__D:                    // 0x52
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__E:                    // 0x53
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__H:                    // 0x54
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__L:                    // 0x55
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d:                          // 0x56
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__2__INDIRECT_IX_d_OR_IY_d__A:                    // 0x57
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__B:                    // 0x58
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__C:                    // 0x59
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__D:                    // 0x5a
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__E:                    // 0x5b
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__H:                    // 0x5c
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__L:                    // 0x5d
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d:                          // 0x5e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__3__INDIRECT_IX_d_OR_IY_d__A:                    // 0x5f
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = OpcodeSize },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                3,
            };


        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__B:                    // 0x60
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__C:                    // 0x61
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__D:                    // 0x62
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__E:                    // 0x63
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__H:                    // 0x64
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__L:                    // 0x65
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d:                          // 0x66
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__4__INDIRECT_IX_d_OR_IY_d__A:                    // 0x67
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__B:                    // 0x68
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__C:                    // 0x69
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__D:                    // 0x6a
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__E:                    // 0x6b
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__H:                    // 0x6c
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__L:                    // 0x6d
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d:                          // 0x6e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__5__INDIRECT_IX_d_OR_IY_d__A:                    // 0x6f
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__B:                    // 0x70
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__C:                    // 0x71
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__D:                    // 0x72
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__E:                    // 0x73
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__H:                    // 0x74
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__L:                    // 0x75
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d:                          // 0x76
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__6__INDIRECT_IX_d_OR_IY_d__A:                    // 0x77
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__B:                    // 0x78
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__C:                    // 0x79
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__D:                    // 0x7a
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__E:                    // 0x7b
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__H:                    // 0x7c
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__L:                    // 0x7d
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d:                          // 0x7e
            return {
                    Instruction::BIT,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__BIT__7__INDIRECT_IX_d_OR_IY_d__A:                    // 0x7f
            return {
                Instruction::BIT,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__B:                    // 0x80
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__C:                    // 0x81
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__D:                    // 0x82
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__E:                    // 0x83
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__H:                    // 0x84
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__L:                    // 0x85
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d:                          // 0x86
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__0__INDIRECT_IX_d_OR_IY_d__A:                    // 0x87
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__B:                    // 0x88
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__C:                    // 0x89
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__D:                    // 0x8a
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__E:                    // 0x8b
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__H:                    // 0x8c
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__L:                    // 0x8d
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d:                          // 0x8e
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__1__INDIRECT_IX_d_OR_IY_d__A:                    // 0x8f
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__B:                    // 0x90
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__C:                    // 0x91
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__D:                    // 0x92
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__E:                    // 0x93
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__H:                    // 0x94
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__L:                    // 0x95
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d:                          // 0x96
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__2__INDIRECT_IX_d_OR_IY_d__A:                    // 0x97
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__B:                    // 0x98
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__C:                    // 0x99
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__D:                    // 0x9a
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__E:                    // 0x9b
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__H:                    // 0x9c
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__L:                    // 0x9d
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d:                          // 0x9e
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__3__INDIRECT_IX_d_OR_IY_d__A:                    // 0x9f
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__B:                    // 0xa0
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__C:                    // 0xa1
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__D:                    // 0xa2
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__E:                    // 0xa3
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__H:                    // 0xa4
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__L:                    // 0xa5
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d:                          // 0xa6
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__4__INDIRECT_IX_d_OR_IY_d__A:                    // 0xa7
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__B:                    // 0xa8
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__C:                    // 0xa9
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__D:                    // 0xaa
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__E:                    // 0xab
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__H:                    // 0xac
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__L:                    // 0xad
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d:                          // 0xae
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__5__INDIRECT_IX_d_OR_IY_d__A:                    // 0xaf
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__B:                    // 0xb0
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__C:                    // 0xb1
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__D:                    // 0xb2
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__E:                    // 0xb3
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__H:                    // 0xb4
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__L:                    // 0xb5
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d:                          // 0xb6
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__6__INDIRECT_IX_d_OR_IY_d__A:                    // 0xb7
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__B:                    // 0xb8
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__C:                    // 0xb9
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__D:                    // 0xba
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__E:                    // 0xbb
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__H:                    // 0xbc
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__L:                    // 0xbd
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d:                          // 0xbe
            return {
                    Instruction::RES,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__RES__7__INDIRECT_IX_d_OR_IY_d__A:                    // 0xbf
            return {
                Instruction::RES,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__B:                    // 0xc0
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__C:                    // 0xc1
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__D:                    // 0xc2
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__E:                    // 0xc3
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__H:                    // 0xc4
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__L:                    // 0xc5
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

       case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d:                          // 0xc6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__0__INDIRECT_IX_d_OR_IY_d__A:                    // 0xc7
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__B:                    // 0xc8
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__C:                    // 0xc9
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__D:                    // 0xca
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__E:                    // 0xcb
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__H:                    // 0xcc
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__L:                    // 0xcd
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d:                          // 0xce
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__1__INDIRECT_IX_d_OR_IY_d__A:                    // 0xcf
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__B:                    // 0xd0
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__C:                    // 0xd1
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__D:                    // 0xd2
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__E:                    // 0xd3
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__H:                    // 0xd4
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__L:                    // 0xd5
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d:                          // 0xd6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__2__INDIRECT_IX_d_OR_IY_d__A:                    // 0xd7
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__B:                    // 0xd8
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__C:                    // 0xd9
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__D:                    // 0xda
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__E:                    // 0xdb
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__H:                    // 0xdc
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__L:                    // 0xdd
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d:                          // 0xde
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__3__INDIRECT_IX_d_OR_IY_d__A:                    // 0xdf
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__B:                    // 0xe0
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__C:                    // 0xe1
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__D:                    // 0xe2
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__E:                    // 0xe3
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__H:                    // 0xe4
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__L:                    // 0xe5
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d:                          // 0xe6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__4__INDIRECT_IX_d_OR_IY_d__A:                    // 0xe7
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__B:                    // 0xe8
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__C:                    // 0xe9
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__D:                    // 0xea
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__E:                    // 0xeb
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__H:                    // 0xec
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__L:                    // 0xed
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d:                          // 0xee
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__5__INDIRECT_IX_d_OR_IY_d__A:                    // 0xef
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__B:                    // 0xf0
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__C:                    // 0xf1
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__D:                    // 0xf2
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__E:                    // 0xf3
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__H:                    // 0xf4
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__L:                    // 0xf5
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d:                          // 0xf6
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__6__INDIRECT_IX_d_OR_IY_d__A:                    // 0xf7
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };


        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__B:                    // 0xf8
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__C:                    // 0xf9
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__D:                    // 0xfa
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__E:                    // 0xfb
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__H:                    // 0xfc
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__L:                    // 0xfd
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                OpcodeSize
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d:                          // 0x06
            return {
                    Instruction::SET,
                    {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                            {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    },
                    OpcodeSize,
            };

        case Z80__DD_OR_FD__CB__SET__7__INDIRECT_IX_d_OR_IY_d__A:                    // 0xff
            return {
                Instruction::SET,
                {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                OpcodeSize
            };

    }

    std::cerr << "disassembly of opcode " << (Register16::IX == reg ? "0xdd " : "0xfd ") << "0xcb " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(*machineCode) << " not yet implemented\n" << std::setfill('0') << std::dec;
    return {
            Instruction::NOP,
            {},
            1,
    };
}
