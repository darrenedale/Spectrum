//
// Created by darren on 17/03/2021.
//

#include <format>

#include "disassembler.h"
#include "../opcodes/opcodes.h"
#include "../z80.h"
#include "../../util/debug.h"

using namespace Z80::Assembly;

using Z80::Register8;
using Z80::Register16;
using Z80::UnsignedWord;
using Z80::UnsignedByte;
using Z80::SignedByte;

namespace
{
    UnsignedWord readUnsignedWord(const UnsignedByte * memory)
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
    void fetchInstructionMachineCode(const Z80::Z80::MemoryType * memory, const int address, UnsignedByte machineCode[4])
    {
        if (const auto bytesAvailable = memory->addressableSize() - address; bytesAvailable < 4) {
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

    while (0 != maxCount && address < m_memory->addressableSize()) {
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

    if (m_pc >= m_memory->addressableSize()) {
        return {
            .instruction = Instruction::NOP,
            .operands = {},
            .size = 1,
        };
    }

    fetchInstructionMachineCode(m_memory, m_pc, machineCode);
    auto mnemonic = disassembleOne(machineCode);
    m_pc += mnemonic.size;
    return mnemonic;
}

Mnemonic Disassembler::disassembleOne(const UnsignedByte * machineCode)
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

Mnemonic Disassembler::disassembleOnePlain(const UnsignedByte * machineCode)
{
    switch (*machineCode) {
        case Opcodes::Z80_Plain_Nop:                // 0x00
            return {
                .instruction = Instruction::NOP,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_Bc_Nn:         // 0x01
            return {
                .instruction = Instruction::LD,
                .operands = {
                        {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                        {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };
        
        case Opcodes::Z80_Plain_Ld_IndirectBc_A:                // 0x02
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::BC,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                     },
                    .size = 1,
            };
        
        case Opcodes::Z80_Plain_Inc_Bc:                // 0x03
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_B:                // 0x04
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_B:                // 0x05
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_N:                // 0x06
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Rlca:                // 0x07
            return {
                    .instruction = Instruction::RLCA,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ex_Af_AfShadow:                // 0x08
            return {
                    .instruction = Instruction::EX,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::AF,},
                             {.mode = AddressingMode::Register16, .register16 = Register16::AFShadow,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_Hl_Bc:                // 0x09
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_IndirectBc:                // 0x0a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::BC,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_Bc:                // 0x0b
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_C:                // 0x0c
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_C:                // 0x0d
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_N:                // 0x0e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Rrca:                // 0x0f
            return {
                    .instruction = Instruction::RRCA,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Djnz_d:                // 0x10
            return {
                .instruction = Instruction::DJNZ,
                .operands = {
                        {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),}
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_De_Nn:                // 0x11
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                             {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                     },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Ld_IndirectDe_A:                // 0x12
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::DE,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_De:                // 0x13
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                            { .mode = AddressingMode::Register16, .register16 = Register16::DE,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_D:                // 0x14
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_D:                // 0x15
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_N:                // 0x16
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Rla:                // 0x17
            return {
                .instruction = Instruction::RLA,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jr_d:                // 0x18
            return {
                .instruction = Instruction::JR,
                .operands = {
                        {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                },
                .size = 2,
            };

        case Opcodes::Z80_Plain_Add_Hl_De:                // 0x19
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            { .mode = AddressingMode::Register16, .register16 =Register16::HL,},
                            { .mode = AddressingMode::Register16, .register16 =Register16::DE,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_IndirectDe:                // 0x1a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 =Register8::A,},
                            { .mode = AddressingMode::Register16Indirect, .register16 =Register16::DE,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_De:                // 0x1b
            return {
                .instruction = Instruction::DEC,
                .operands = {
                    { .mode = AddressingMode::Register16, .register16 = Register16::DE,}
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_E:                // 0x1c
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_E:                // 0x1d
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_N:                // 0x1e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Rra:                // 0x1f
            return {
                .instruction = Instruction::RRA,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jr_Nz_d:                // 0x20
            return {
                    .instruction = Instruction::JRNZ,
                    .operands = {
                            {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Ld_Hl_Nn:                // 0x21
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register16, .register16 = Register16::HL,},
                            { .mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),}
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Ld_IndirectNn_Hl:                // 0x22
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Inc_Hl:                // 0x23
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_H:                // 0x24
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_H:                // 0x25
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_N:                // 0x26
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Daa:                // 0x27
            return {
                .instruction = Instruction::DAA,
                .operands = {},
                .size = 1,
            };
            
        case Opcodes::Z80_Plain_Jr_Z_d:                // 0x28
            return {
                    .instruction = Instruction::JRZ,
                    .operands = {
                            {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Add_Hl_Hl:                // 0x29
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            { .mode = AddressingMode::Register16, .register16 =Register16::HL,},
                            { .mode = AddressingMode::Register16, .register16 =Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_Hl_IndirectNn:                // 0x2a
            return {
                .instruction = Instruction::LD,
                .operands = {
                        {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                        {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };

        case Opcodes::Z80_Plain_Dec_Hl:                // 0x2b
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                            { .mode = AddressingMode::Register16, .register16 = Register16::HL,}
                    },
                    .size = 1,
            };
            
        case Opcodes::Z80_Plain_Inc_L:                // 0x2c
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                     },
                    .size = 1,
            };
            
        case Opcodes::Z80_Plain_Dec_L:                // 0x2d
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_N:                // 0x2e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                             {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                     },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Cpl:                // 0x2f
            return {
                .instruction = Instruction::CPL,
                .operands = {},
                .size = 1,
            };
            
        case Opcodes::Z80_Plain_Jr_Nc_d:                // 0x30
            return {
                .instruction = Instruction::JRNC,
                .operands = {
                        {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)), },
                },
                .size = 2,
            };

        case Opcodes::Z80_Plain_Ld_Sp_Nn:                // 0x31
            return {
                .instruction = Instruction::LD,
                .operands = {
                    { .mode = AddressingMode::Register16, .register16 = Register16::SP,},
                    { .mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectNn_A:                // 0x32
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Inc_Sp:                // 0x33
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::SP,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_IndirectHl:                // 0x34
            return {
                    .instruction = Instruction::INC,
                    .operands = {
                            { .mode = AddressingMode::Register16Indirect, .register16 =Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_IndirectHl:                // 0x35
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                            { .mode = AddressingMode::Register16Indirect, .register16 =Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_N:                // 0x36
            return {
                .instruction = Instruction::LD,
                .operands = {
                    { .mode = AddressingMode::Register16Indirect, .register16 =Register16::HL,},
                    { .mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                },
                .size = 2,
            };

        case Opcodes::Z80_Plain_Scf:                // 0x37
            return {
                .instruction = Instruction::SCF,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jr_C_d:                // 0x38
            return {
                    .instruction = Instruction::JRC,
                    .operands = {
                            {.mode = AddressingMode::Relative, .signedByte = static_cast<SignedByte>(*(machineCode + 1)),},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Add_Hl_Sp:                // 0x39
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                             {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register16, .register16 = Register16::SP,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_IndirectNn:                // 0x3a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Dec_Sp:                // 0x3b
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                            { .mode = AddressingMode::Register16, .register16 = Register16::SP,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Inc_A:                // 0x3c
            return {
                .instruction = Instruction::INC,
                .operands = {
                 {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Dec_A:                // 0x3d
            return {
                    .instruction = Instruction::DEC,
                    .operands = {
                             {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_N:                // 0x3e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 =Register8::A,},
                            { .mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Ccf:                // 0x3f
            return {
                .instruction = Instruction::CCF,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_B:                // 0x40
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_C:                // 0x41
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_D:                // 0x42
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_E:                // 0x43
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_H:                // 0x44
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_L:                // 0x45
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_IndirectHl:                // 0x46
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_B_A:                // 0x47
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_B:                // 0x48
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_C:                // 0x49
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_D:                // 0x4a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_E:                // 0x4b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_H:                // 0x4c
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_L:                // 0x4d
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_IndirectHl:                // 0x4e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_C_A:                // 0x4f
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_B:                // 0x50
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_C:                // 0x51
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_D:                // 0x52
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_E:                // 0x53
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_H:                // 0x54
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_L:                // 0x55
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_IndirectHl:                // 0x56
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_D_A:                // 0x57
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_B:                // 0x58
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_C:                // 0x59
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_D:                // 0x5a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_E:                // 0x5b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_H:                // 0x5c
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_L:                // 0x5d
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_IndirectHl:                // 0x5e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_E_A:                // 0x5f
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_B:                // 0x60
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_C:                // 0x61
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_D:                // 0x62
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_E:                // 0x63
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_H:                // 0x64
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_L:                // 0x65
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_IndirectHl:                // 0x66
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_H_A:                // 0x67
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_B:                // 0x68
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_C:                // 0x69
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_D:                // 0x6a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_E:                // 0x6b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_H:                // 0x6c
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_L:                // 0x6d
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_IndirectHl:                // 0x6e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_L_A:                // 0x6f
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_B:                // 0x70
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                             {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                             {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                     },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_C:                // 0x71
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_D:                // 0x72
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_E:                // 0x73
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_H:                // 0x74
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_L:                // 0x75
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Halt:                // 0x76
            return {
                .instruction = Instruction::HALT,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_IndirectHl_A:                // 0x77
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_B:                // 0x78
            return {
                .instruction = Instruction::LD,
                .operands = {
                        {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                        {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_C:                // 0x79
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_D:                // 0x7a
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_E:                // 0x7b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_H:                // 0x7c
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_L:                // 0x7d
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_IndirectHl:                // 0x7e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_A_A:                // 0x7f
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_B:                // 0x80
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_C:                // 0x81
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_D:                // 0x82
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_E:                // 0x83
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_H:                // 0x84
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_L:                // 0x85
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_IndirectHl:                // 0x86
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_A:                // 0x87
            return {
                    .instruction = Instruction::ADD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_B:                // 0x88
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_C:                // 0x89
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_D:                // 0x8a
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_E:                // 0x8b
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_H:                // 0x8c
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_L:                // 0x8d
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_IndirectHl:                // 0x8e
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Adc_A_A:                // 0x8f
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_B:                // 0x90
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_C:                // 0x91
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_D:                // 0x92
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_E:                // 0x93
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_H:                // 0x94
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_L:                // 0x95
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_IndirectHl:                // 0x96
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_A:                // 0x97
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_B:                // 0x98
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_C:                // 0x99
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_D:                // 0x9a
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_E:                // 0x9b
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_H:                // 0x9c
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_L:                // 0x9d
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_IndirectHl:                // 0x9e
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Sbc_A_A:                // 0x9f
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_B:                // 0xa0
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_C:                // 0xa1
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_D:                // 0xa2
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_E:                // 0xa3
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_H:                // 0xa4
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_L:                // 0xa5
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_IndirectHl:                // 0xa6
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_And_A:                // 0xa7
            return {
                .instruction = Instruction::AND,
                .operands = {
                        {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_B:                // 0xa8
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_C:                // 0xa9
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_D:                // 0xaa
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_E:                // 0xab
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_H:                // 0xac
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_L:                // 0xad
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_IndirectHl:                // 0xae
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Xor_A:                // 0xaf
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_B:                // 0xb0
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_C:                // 0xb1
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_D:                // 0xb2
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_E:                // 0xb3
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_H:                // 0xb4
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_L:                // 0xb5
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_IndirectHl:                // 0xb6
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Or_A:                // 0xb7
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_B:                // 0xb8
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_C:                // 0xb9
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_D:                // 0xba
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_E:                // 0xbb
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_H:                // 0xbc
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_L:                // 0xbd
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_IndirectHl:                // 0xbe
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Cp_A:                // 0xbf
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,}
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ret_Nz:                // 0xc0
            return {
                .instruction = Instruction::RETNZ,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Pop_Bc:                // 0xc1
            return {
                .instruction = Instruction::POP,
                .operands = {
                     {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_Nz_Nn:                // 0xc2
            return {
                    .instruction = Instruction::JPNZ,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Jp_Nn:                // 0xc3
            return {
                    .instruction = Instruction::JP,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord((machineCode + 1)), },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Call_Nz_Nn:                // 0xc4
            return {
                    .instruction = Instruction::CALLNZ,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Push_Bc:                // 0xc5
            return {
                    .instruction = Instruction::PUSH,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Add_A_N:                // 0xc6
            return {
                .instruction = Instruction::ADD,
                .operands = {
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                },
                .size = 2,
            };

        case Opcodes::Z80_Plain_Rst_00:                // 0xc7
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                        // NOTE no need for endian conversion, both bytes are the same
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = 0x0000,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ret_Z:                // 0xc8
            return {
                .instruction = Instruction::RETZ,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ret:                // 0xc9
            return {
                    .instruction = Instruction::RET,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_Z_Nn:                // 0xca
            return {
                    .instruction = Instruction::JPZ,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Prefix_Cb:                // 0xcb
            // NOTE should never get here
            return disassembleOneCb(machineCode + 1);

        case Opcodes::Z80_Plain_Call_Z_Nn:                // 0xcc
            return {
                    .instruction = Instruction::CALLZ,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Call_Nn:                // 0xcd
            return {
                .instruction = Instruction::CALL,
                .operands = {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };

        case Opcodes::Z80_Plain_Adc_A_N:                // 0xce
            return {
                .instruction = Instruction::ADC,
                .operands = {
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                },
                .size = 2,
            };

        case Opcodes::Z80_Plain_Rst_08:                // 0xcf
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0008),},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ret_Nc:                // 0xd0
            return {
                    .instruction = Instruction::RETNC,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Pop_De:                // 0xd1
            return {
                .instruction = Instruction::POP,
                .operands = {
                     {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_Nc_Nn:                // 0xd2
            return {
                    .instruction = Instruction::JPNC,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Out_IndirectN_A:                // 0xd3
            return {
                .instruction = Instruction::OUT,
                .operands = {
                    {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = 2,
            };

        case Opcodes::Z80_Plain_Call_Nc_Nn:                // 0xd4
            return {
                .instruction = Instruction::CALLNC,
                .operands = {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };

        case Opcodes::Z80_Plain_Push_De:                // 0xd5
            return {
                .instruction = Instruction::PUSH,
                .operands = {
                    {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Sub_N:                // 0xd6
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Rst_10:                // 0xd7
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0010),},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ret_C:                // 0xd8
            return {
                    .instruction = Instruction::RETC,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Exx:                // 0xd9
            return {
                .instruction = Instruction::EXX,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_C_Nn:                // 0xda
            return {
                    .instruction = Instruction::JPC,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };
        
        case Opcodes::Z80_Plain_In_A_IndirectN:                // 0xdb
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };
            
        case Opcodes::Z80_Plain_Call_C_Nn:                // 0xdc
            return {
                    .instruction = Instruction::CALLC,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };
            
        case Opcodes::Z80_Plain_Prefix_Dd:                // 0xdd
            // NOTE should never get here
            return disassembleOneDdOrFd(Register16::IX, machineCode + 1);
            
        case Opcodes::Z80_Plain_Sbc_A_N:                // 0xde
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };
            
        case Opcodes::Z80_Plain_Rst_18:                // 0xdf
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0018),},
                    },
                    .size = 1,
            };
            
        case Opcodes::Z80_Plain_Ret_Po:                // 0xe0
            return {
                    .instruction = Instruction::RETPO,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Pop_Hl:                // 0xe1
            return {
                .instruction = Instruction::POP,
                .operands = {
                    {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_Po_Nn:                // 0xe2
            return {
                    .instruction = Instruction::JPPO,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Ex_IndirectSp_Hl:                // 0xe3
            return {
                    .instruction = Instruction::EX,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::SP,},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Call_Po_Nn:                // 0xe4
            return {
                    .instruction = Instruction::CALLPO,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };

        case Opcodes::Z80_Plain_Push_Hl:                // 0xe5
            return {
                .instruction = Instruction::PUSH,
                .operands = {
                    {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_And_N:                // 0xe6
            return {
                    .instruction = Instruction::AND,
                    .operands = {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Plain_Rst_20:                // 0xe7
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0020),},
                    },
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Ret_Pe:                // 0xe8
            return {
                    .instruction = Instruction::RETPE,
                    .operands = {},
                    .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_IndirectHl:                // 0xe9
            return {
                .instruction = Instruction::JPM,
                .operands = {
                    {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                },
                .size = 1,
            };

        case Opcodes::Z80_Plain_Jp_Pe_Nn:                // 0xea
            return {
                .instruction = Instruction::JPPE,
                .operands{
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };
            
        case Opcodes::Z80_Plain_Ex_De_Hl:                // 0xeb
            return {
                    .instruction = Instruction::EX,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE,},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };
        
        case Opcodes::Z80_Plain_Call_Pe_Nn:                // 0xec
            return {
                .instruction = Instruction::CALLPE,
                .operands = {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };
            
        case Opcodes::Z80_Plain_Prefix_Ed:                // 0xed
            // NOTE should never get here
            return disassembleOneEd(machineCode + 1);
            
        case Opcodes::Z80_Plain_Xor_N:                // 0xee
            return {
                    .instruction = Instruction::XOR,
                    .operands = {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };
            
        case Opcodes::Z80_Plain_Rst_28:                // 0xef
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0028),},
                    },
                    .size = 1,
            };
        
        case Opcodes::Z80_Plain_Ret_P:                // 0xf0
            return {
                    .instruction = Instruction::RETP,
                    .operands = {},
                    .size = 1,
            };
        
        case Opcodes::Z80_Plain_Pop_Af:                // 0xf1
            return {
                .instruction = Instruction::POP,
                .operands = {
                    {.mode = AddressingMode::Register16, .register16 = Register16::AF,},
                },
                .size = 1,
            };
        
        case Opcodes::Z80_Plain_Jp_P_Nn:                // 0xf2
            return {
                .instruction = Instruction::JPP,
                .operands = {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };
            
        case Opcodes::Z80_Plain_Di:                // 0xf3
            return {
                .instruction = Instruction::DI,
                .operands = {},
                .size = 1,
            };
            
        case Opcodes::Z80_Plain_Call_P_Nn:                // 0xf4
            return {
                    .instruction = Instruction::CALLP,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                    .size = 3,
            };
            
        case Opcodes::Z80_Plain_Push_Af:                // 0xf5
            return {
                    .instruction = Instruction::PUSH,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::AF,},
                    },
                    .size = 1,
            };
            
        case Opcodes::Z80_Plain_Or_N:                // 0xf6
            return {
                    .instruction = Instruction::OR,
                    .operands = {
                            {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };
            
        case Opcodes::Z80_Plain_Rst_30:                // 0xf7
            return {
                    .instruction = Instruction::RST,
                    .operands = {
                            {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0030),},
                    },
                    .size = 1,
            };
            
        case Opcodes::Z80_Plain_Ret_M:                // 0xf8
            return {
                .instruction = Instruction::RETM,
                .operands = {},
                .size = 1,
            };

        case Opcodes::Z80_Plain_Ld_Sp_Hl:                // 0xf9
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP,},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL,},
                    },
                    .size = 1,
            };
            
        case Opcodes::Z80_Plain_Jp_M_Nn:                // 0xfa
            return {
                .instruction = Instruction::JPM,
                .operands = {
                        {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                    },
                .size = 3,
            };
            
        case Opcodes::Z80_Plain_Ei:                // 0xfb
            return {
                .instruction = Instruction::EI,
                .operands = {},
                .size = 1,
            };
            
        case Opcodes::Z80_Plain_Call_M_Nn:                // 0xfc
            return {
                .instruction = Instruction::CALLM,
                .operands = {
                    {.mode = AddressingMode::ImmediateExtended, .unsignedWord = readUnsignedWord(machineCode + 1),},
                },
                .size = 3,
            };
            
        case Opcodes::Z80_Plain_Prefix_Fd:                // 0xfd
            // NOTE should never get here
            return disassembleOneDdOrFd(Register16::IY, machineCode + 1);
            
        case Opcodes::Z80_Plain_Cp_N:                // 0xfe
            return {
                    .instruction = Instruction::CP,
                    .operands = {
                        {.mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1),},
                    },
                    .size = 2,
            };
            
        case Opcodes::Z80_Plain_Rst_38:                // 0xff
            return {
                .instruction = Instruction::RST,
                .operands = {
                        {.mode = AddressingMode::ImmediateExtended, .unsignedWord = hostToZ80ByteOrder(0x0038),},
                },
                .size = 1,
            };

        default:
            Util::debugln("disassembly of opcode {:#02x} not yet implemented", static_cast<std::uint16_t>(*machineCode));
            break;
    }

    return {
        .instruction = Instruction::NOP,
        .operands = {},
        .size = 1,
    };
}

Mnemonic Disassembler::disassembleOneCb(const UnsignedByte * machineCode)
{
    // NOTE all 0xcb prefix opcodes are this size
    static constexpr const UnsignedByte OpcodeSize = 2;

    switch (*machineCode) {
        case Opcodes::Z80_Cb_Rlc_B:					// 0x00
            return {
                .instruction = Instruction::RLC,
                .operands = {
                        {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_C:					// 0x01
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_D:					// 0x02
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_E:					// 0x03
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_H:					// 0x04
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_L:					// 0x05
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_IndirectHl:	// 0x06
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rlc_A:					// 0x07
            return {
                    .instruction = Instruction::RLC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_B:					// 0x08
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_C:					// 0x09
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_D:					// 0x0a
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_E:					// 0x0b
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_H:					// 0x0c
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_L:					// 0x0d
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_IndirectHl:	// 0x0e
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rrc_A:					// 0x0f
            return {
                    .instruction = Instruction::RRC,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_B:					// 0x10
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_C:					// 0x11
            return {
                .instruction = Instruction::RL,
                .operands = {
                        {.mode = AddressingMode::Register8, .register8 = Register8::C,}
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_D:					// 0x12
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_E:					// 0x13
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_H:					// 0x14
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_L:					// 0x15
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_IndirectHl:		// 0x16
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rl_A:					// 0x17
            return {
                    .instruction = Instruction::RL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_B:					// 0x18
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_C:					// 0x19
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_D:					// 0x1a
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_E:					// 0x1b
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_H:					// 0x1c
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_L:					// 0x1d
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_IndirectHl:		// 0x1e
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Rr_A:					// 0x1f
            return {
                    .instruction = Instruction::RR,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_B:					// 0x20
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_C:					// 0x21
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_D:					// 0x22
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_E:					// 0x23
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_H:					// 0x24
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_L:					// 0x25
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_IndirectHl:	// 0x26
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sla_A:					// 0x27
            return {
                    .instruction = Instruction::SLA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_B:					// 0x28
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_C:					// 0x29
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_D:					// 0x2a
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_E:					// 0x2b
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_H:					// 0x2c
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_L:					// 0x2d
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_IndirectHl:	// 0x2e
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sra_A:					// 0x2f
            return {
                    .instruction = Instruction::SRA,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_B:					// 0x30
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_C:					// 0x31
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_D:					// 0x32
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_E:					// 0x33
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_H:					// 0x34
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_L:					// 0x35
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_IndirectHl:	// 0x36
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Sll_A:					// 0x37
            return {
                    .instruction = Instruction::SLL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_B:					// 0x38
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_C:					// 0x39
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_D:					// 0x3a
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_E:					// 0x3b
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_H:					// 0x3c
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_L:					// 0x3d
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_IndirectHl:	// 0x3e
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Srl_A:					// 0x3f
            return {
                    .instruction = Instruction::SRL,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_B:					// 0x40
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_C:					// 0x41
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_D:					// 0x42
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_E:					// 0x43
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_H:					// 0x44
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_L:					// 0x45
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_IndirectHl:	// 0x46
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_0_A:					// 0x47
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_B:					// 0x48
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_C:					// 0x49
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_D:					// 0x4a
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_E:					// 0x4b
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_H:					// 0x4c
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_L:					// 0x4d
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_IndirectHl:	// 0x4e
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_1_A:					// 0x4f
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_B:					// 0x50
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_C:					// 0x51
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_D:					// 0x52
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_E:					// 0x53
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_H:					// 0x54
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_L:					// 0x55
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_IndirectHl:	// 0x56
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_2_A:					// 0x57
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_B:					// 0x58
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_C:					// 0x59
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_D:					// 0x5a
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_E:					// 0x5b
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_H:					// 0x5c
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_L:					// 0x5d
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_IndirectHl:	// 0x5e
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_3_A:					// 0x5f
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_B:					// 0x60
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_C:					// 0x61
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_D:					// 0x62
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_E:					// 0x63
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_H:					// 0x64
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_L:					// 0x65
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_IndirectHl:	// 0x66
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_4_A:					// 0x67
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_B:					// 0x68
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_C:					// 0x69
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_D:					// 0x6a
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_E:					// 0x6b
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_H:					// 0x6c
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_L:					// 0x6d
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_IndirectHl:	// 0x6e
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_5_A:					// 0x6f
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_B:					// 0x70
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_C:					// 0x71
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_D:					// 0x72
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_E:					// 0x73
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_H:					// 0x74
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_L:					// 0x75
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_IndirectHl:	// 0x76
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_6_A:					// 0x77
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_B:					// 0x78
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_C:					// 0x79
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_D:					// 0x7a
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_E:					// 0x7b
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_H:					// 0x7c
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_L:					// 0x7d
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_IndirectHl:	// 0x7e
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Bit_7_A:					// 0x7f
            return {
                    .instruction = Instruction::BIT,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_B:					// 0x80
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_C:					// 0x81
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_D:					// 0x82
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_E:					// 0x83
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_H:					// 0x84
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_L:					// 0x85
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_IndirectHl:	// 0x86
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_0_A:					// 0x87
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_B:					// 0x88
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_C:					// 0x89
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_D:					// 0x8a
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_E:					// 0x8b
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_H:					// 0x8c
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_L:					// 0x8d
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_IndirectHl:	// 0x8e
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_1_A:					// 0x8f
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_B:					// 0x90
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_C:					// 0x91
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_D:					// 0x92
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_E:					// 0x93
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_H:					// 0x94
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_L:					// 0x95
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_IndirectHl:	// 0x96
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_2_A:					// 0x97
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_B:					// 0x98
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_C:					// 0x99
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_D:					// 0x9a
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_E:					// 0x9b
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_H:					// 0x9c
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_L:					// 0x9d
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_IndirectHl:	// 0x9e
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_3_A:					// 0x9f
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_B:					// 0xa0
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_C:					// 0xa1
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_D:					// 0xa2
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_E:					// 0xa3
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_H:					// 0xa4
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_L:					// 0xa5
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_IndirectHl:	// 0xa6
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_4_A:					// 0xa7
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_B:					// 0xa8
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_C:					// 0xa9
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_D:					// 0xaa
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_E:					// 0xab
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_H:					// 0xac
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_L:					// 0xad
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_IndirectHl:	// 0xae
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_5_A:					// 0xaf
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_B:					// 0xb0
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_C:					// 0xb1
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_D:					// 0xb2
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_E:					// 0xb3
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_H:					// 0xb4
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_L:					// 0xb5
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_IndirectHl:	// 0xb6
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_6_A:					// 0xb7
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_B:					// 0xb8
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_C:					// 0xb9
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_D:					// 0xba
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_E:					// 0xbb
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_H:					// 0xbc
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_L:					// 0xbd
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_IndirectHl:	// 0xbe
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Res_7_A:					// 0xbf
            return {
                    .instruction = Instruction::RES,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_B:					// 0xc0
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_C:					// 0xc1
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_D:					// 0xc2
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_E:					// 0xc3
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_H:					// 0xc4
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_L:					// 0xc5
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_IndirectHl:	// 0xc6
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_0_A:					// 0xc7
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 0,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_B:					// 0xc8
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_C:					// 0xc9
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_D:					// 0xca
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_E:					// 0xcb
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_H:					// 0xcc
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_L:					// 0xcd
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_IndirectHl:	// 0xce
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_1_A:					// 0xcf
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 1,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_B:					// 0xd0
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_C:					// 0xd1
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_D:					// 0xd2
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_E:					// 0xd3
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_H:					// 0xd4
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_L:					// 0xd5
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_IndirectHl:	// 0xd6
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_2_A:					// 0xd7
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 2,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_B:					// 0xd8
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_C:					// 0xd9
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_D:					// 0xda
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_E:					// 0xdb
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_H:					// 0xdc
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_L:					// 0xdd
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_IndirectHl:	// 0xde
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_3_A:					// 0xdf
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 3,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_B:					// 0xe0
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_C:					// 0xe1
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_D:					// 0xe2
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_E:					// 0xe3
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_H:					// 0xe4
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_L:					// 0xe5
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_IndirectHl:	// 0xe6
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_4_A:					// 0xe7
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 4,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_B:					// 0xe8
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_C:					// 0xe9
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_D:					// 0xea
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_E:					// 0xeb
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_H:					// 0xec
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_L:					// 0xed
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_IndirectHl:	// 0xee
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_5_A:					// 0xef
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 5,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_B:					// 0xf0
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_C:					// 0xf1
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_D:					// 0xf2
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_E:					// 0xf3
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_H:					// 0xf4
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_L:					// 0xf5
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_IndirectHl:	// 0xf6
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_6_A:					// 0xf7
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 6,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_B:					// 0xf8
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_C:					// 0xf9
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_D:					// 0xfa
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_E:					// 0xfb
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_H:					// 0xfc
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_L:					// 0xfd
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_IndirectHl:	// 0xfe
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register16Indirect, .register16 = Register16::HL,},
                    },
                    .size = OpcodeSize,
            };

        case Opcodes::Z80_Cb_Set_7_A:					// 0xff
            return {
                    .instruction = Instruction::SET,
                    .operands = {
                            {.mode = AddressingMode::Bit, .unsignedByte = 7,},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                    },
                    .size = OpcodeSize,
            };

        default:
            Util::debugln("disassembly of opcode 0xcb {:#02x} not yet implemented", static_cast<std::uint16_t>(*machineCode));
            break;
    }


    return {
        .instruction = Instruction::NOP,
        .operands = {},
        .size = 1,
    };
}

Mnemonic Disassembler::disassembleOneEd(const UnsignedByte * machineCode)
{
    switch (*machineCode)
    {
        case Opcodes::Z80_Ed_In_B_IndirectC:            // 0x40
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::B},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_B:            // 0x41
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::B},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Sbc_Hl_Bc:                    // 0x42
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_IndirectNn_Bc:        // 0x43
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg:                                // 0x44
            return {
                .instruction = Instruction::NEG,
                .operands = {},
                .size = 2,
            };

        case Opcodes::Z80_Ed_Retn:                            // 0x45
            return {
                    .instruction = Instruction::RETN,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_0:                            // 0x46
            return {
                    .instruction = Instruction::IM0,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_I_A:                        // 0x47
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::I},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_In_C_IndirectC:            // 0x48
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_C:            // 0x49
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Adc_Hl_Bc:                    // 0x4a
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_Bc_IndirectNn:        // 0x4b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::BC},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x4c:                // 0x4c
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Reti:                            // 0x4d
            return {
                    .instruction = Instruction::RETI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_0_0xEd_0x4e:            // 0x4e
            return {
                    .instruction = Instruction::IM0,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_R_A:                        // 0x4f
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::R},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_In_D_IndirectC:            // 0x50
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::D},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_D:            // 0x51
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::D},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Sbc_Hl_De:                    // 0x52
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_IndirectNn_De:        // 0x53
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x54:                // 0x54
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Retn_0xEd_0x55:            // 0x55
            return {
                    .instruction = Instruction::RETN,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_1:                            // 0x56
            return {
                    .instruction = Instruction::IM1,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_A_I:                        // 0x57
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                            {.mode = AddressingMode::Register8, .register8 = Register8::I},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_In_E_IndirectC:            // 0x58
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::E},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_E:            // 0x59
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::E},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Adc_Hl_De:                    // 0x5a
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_De_IndirectNn:        // 0x5b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::DE},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x5c:                // 0x5c
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Reti_0xEd_0x5d:            // 0x5d
            return {
                    .instruction = Instruction::RETI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_2:                            // 0x5e
            return {
                    .instruction = Instruction::IM2,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_A_R:                        // 0x5f
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                            {.mode = AddressingMode::Register8, .register8 = Register8::R},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_In_H_IndirectC:            // 0x60
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::H},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_H:            // 0x61
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::H},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Sbc_Hl_Hl:                    // 0x62
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_IndirectNn_Hl:        // 0x63
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x64:                // 0x64
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Retn_0xEd_0x65:            // 0x65
            return {
                    .instruction = Instruction::RETN,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_0_0xEd_0x66:            // 0x66
            return {
                    .instruction = Instruction::IM0,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Rrd:                                // 0x67
            return {
                    .instruction = Instruction::RRD,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_In_L_IndirectC:            // 0x68
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::L},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_L:            // 0x69
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::L},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Adc_Hl_Hl:                    // 0x6a
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_Hl_IndirectNn:        // 0x6b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x6c:                // 0x6c
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Reti_0xEd_0x6d:            // 0x6d
            return {
                    .instruction = Instruction::RETI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_0_0xEd_0x6e:            // 0x6e
            return {
                    .instruction = Instruction::IM0,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Rld:                                // 0x6f
            return {
                    .instruction = Instruction::RLD,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_In_IndirectC:                // 0x70
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_0:            // 0x71
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Immediate, .unsignedByte = 0x00},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Sbc_Hl_Sp:                    // 0x72
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_IndirectNn_Sp:        // 0x73
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x74:                // 0x74
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Retn_0xEd_0x75:            // 0x75
            return {
                    .instruction = Instruction::RETN,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_1_0xEd_0x76:            // 0x76
            return {
                    .instruction = Instruction::IM1,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_In_A_IndirectC:            // 0x78
            return {
                    .instruction = Instruction::IN,
                    .operands = {
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Out_IndirectC_A:            // 0x79
            return {
                    .instruction = Instruction::OUT,
                    .operands = {
                            {.mode = AddressingMode::Register8Indirect, .register8 = Register8::C},
                            {.mode = AddressingMode::Register8, .register8 = Register8::A},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Adc_Hl_Sp:                    // 0x7a
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::HL},
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                    },
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ld_Sp_IndirectNn:        // 0x7b
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            {.mode = AddressingMode::Register16, .register16 = Register16::SP},
                            {.mode = AddressingMode::Extended, .unsignedWord = readUnsignedWord(machineCode + 1)},
                    },
                    .size = 4,
            };

        case Opcodes::Z80_Ed_Neg_0xEd_0x7c:                // 0x7c
            return {
                    .instruction = Instruction::NEG,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Reti_0xEd_0x7d:            // 0x7d
            return {
                    .instruction = Instruction::RETI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Im_2_0xEd_0x7e:            // 0x7e
            return {
                    .instruction = Instruction::IM2,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ldi:                                // 0xa0
            return {
                    .instruction = Instruction::LDI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Cpi:                                // 0xa1
            return {
                    .instruction = Instruction::CPI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ini:                                // 0xa2
            return {
                    .instruction = Instruction::INI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Outi:                            // 0xa3
            return {
                    .instruction = Instruction::OUTI,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ldd:                                // 0xa8
            return {
                    .instruction = Instruction::LDD,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Cpd:                                // 0xa9
            return {
                    .instruction = Instruction::CPD,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ind:                                // 0xaa
            return {
                    .instruction = Instruction::IND,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Outd:                            // 0xab
            return {
                    .instruction = Instruction::OUTD,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Ldir:                            // 0xb0
            return {
                    .instruction = Instruction::LDIR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Cpir:                            // 0xb1
            return {
                    .instruction = Instruction::CPIR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Inir:                            // 0xb2
            return {
                    .instruction = Instruction::INIR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Otir:                            // 0xb3
            return {
                    .instruction = Instruction::OTIR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Lddr:                            // 0xb8
            return {
                    .instruction = Instruction::LDDR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Cpdr:                            // 0xb9
            return {
                    .instruction = Instruction::CPDR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Indr:                            // 0xba
            return {
                    .instruction = Instruction::INDR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Otdr:                            // 0xbb
            return {
                    .instruction = Instruction::OTDR,
                    .operands = {},
                    .size = 2,
            };

        case Opcodes::Z80_Ed_Nop_0xEd_0x00:                // 0x00
        case Opcodes::Z80_Ed_Nop_0xEd_0x01:                // 0x01
        case Opcodes::Z80_Ed_Nop_0xEd_0x02:                // 0x02
        case Opcodes::Z80_Ed_Nop_0xEd_0x03:                // 0x03
        case Opcodes::Z80_Ed_Nop_0xEd_0x04:                // 0x04
        case Opcodes::Z80_Ed_Nop_0xEd_0x05:                // 0x05
        case Opcodes::Z80_Ed_Nop_0xEd_0x06:                // 0x06
        case Opcodes::Z80_Ed_Nop_0xEd_0x07:                // 0x07
        case Opcodes::Z80_Ed_Nop_0xEd_0x08:                // 0x08
        case Opcodes::Z80_Ed_Nop_0xEd_0x09:                // 0x09
        case Opcodes::Z80_Ed_Nop_0xEd_0x0a:                // 0x0a
        case Opcodes::Z80_Ed_Nop_0xEd_0x0b:                // 0x0b
        case Opcodes::Z80_Ed_Nop_0xEd_0x0c:                // 0x0c
        case Opcodes::Z80_Ed_Nop_0xEd_0x0d:                // 0x0d
        case Opcodes::Z80_Ed_Nop_0xEd_0x0e:                // 0x0e
        case Opcodes::Z80_Ed_Nop_0xEd_0x0f:                // 0x0f
        case Opcodes::Z80_Ed_Nop_0xEd_0x10:                // 0x10
        case Opcodes::Z80_Ed_Nop_0xEd_0x11:                // 0x11
        case Opcodes::Z80_Ed_Nop_0xEd_0x12:                // 0x12
        case Opcodes::Z80_Ed_Nop_0xEd_0x13:                // 0x13
        case Opcodes::Z80_Ed_Nop_0xEd_0x14:                // 0x14
        case Opcodes::Z80_Ed_Nop_0xEd_0x15:                // 0x15
        case Opcodes::Z80_Ed_Nop_0xEd_0x16:                // 0x16
        case Opcodes::Z80_Ed_Nop_0xEd_0x17:                // 0x17
        case Opcodes::Z80_Ed_Nop_0xEd_0x18:                // 0x18
        case Opcodes::Z80_Ed_Nop_0xEd_0x19:                // 0x19
        case Opcodes::Z80_Ed_Nop_0xEd_0x1a:                // 0x1a
        case Opcodes::Z80_Ed_Nop_0xEd_0x1b:                // 0x1b
        case Opcodes::Z80_Ed_Nop_0xEd_0x1c:                // 0x1c
        case Opcodes::Z80_Ed_Nop_0xEd_0x1d:                // 0x1d
        case Opcodes::Z80_Ed_Nop_0xEd_0x1e:                // 0x1e
        case Opcodes::Z80_Ed_Nop_0xEd_0x1f:                // 0x1f
        case Opcodes::Z80_Ed_Nop_0xEd_0x20:                // 0x20
        case Opcodes::Z80_Ed_Nop_0xEd_0x21:                // 0x21
        case Opcodes::Z80_Ed_Nop_0xEd_0x22:                // 0x22
        case Opcodes::Z80_Ed_Nop_0xEd_0x23:                // 0x23
        case Opcodes::Z80_Ed_Nop_0xEd_0x24:                // 0x24
        case Opcodes::Z80_Ed_Nop_0xEd_0x25:                // 0x25
        case Opcodes::Z80_Ed_Nop_0xEd_0x26:                // 0x26
        case Opcodes::Z80_Ed_Nop_0xEd_0x27:                // 0x27
        case Opcodes::Z80_Ed_Nop_0xEd_0x28:                // 0x28
        case Opcodes::Z80_Ed_Nop_0xEd_0x29:                // 0x29
        case Opcodes::Z80_Ed_Nop_0xEd_0x2a:                // 0x2a
        case Opcodes::Z80_Ed_Nop_0xEd_0x2b:                // 0x2b
        case Opcodes::Z80_Ed_Nop_0xEd_0x2c:                // 0x2c
        case Opcodes::Z80_Ed_Nop_0xEd_0x2d:                // 0x2d
        case Opcodes::Z80_Ed_Nop_0xEd_0x2e:                // 0x2e
        case Opcodes::Z80_Ed_Nop_0xEd_0x2f:                // 0x2f
        case Opcodes::Z80_Ed_Nop_0xEd_0x30:                // 0x30
        case Opcodes::Z80_Ed_Nop_0xEd_0x31:                // 0x31
        case Opcodes::Z80_Ed_Nop_0xEd_0x32:                // 0x32
        case Opcodes::Z80_Ed_Nop_0xEd_0x33:                // 0x33
        case Opcodes::Z80_Ed_Nop_0xEd_0x34:                // 0x34
        case Opcodes::Z80_Ed_Nop_0xEd_0x35:                // 0x35
        case Opcodes::Z80_Ed_Nop_0xEd_0x36:                // 0x36
        case Opcodes::Z80_Ed_Nop_0xEd_0x37:                // 0x37
        case Opcodes::Z80_Ed_Nop_0xEd_0x38:                // 0x38
        case Opcodes::Z80_Ed_Nop_0xEd_0x39:                // 0x39
        case Opcodes::Z80_Ed_Nop_0xEd_0x3a:                // 0x3a
        case Opcodes::Z80_Ed_Nop_0xEd_0x3b:                // 0x3b
        case Opcodes::Z80_Ed_Nop_0xEd_0x3c:                // 0x3c
        case Opcodes::Z80_Ed_Nop_0xEd_0x3d:                // 0x3d
        case Opcodes::Z80_Ed_Nop_0xEd_0x3e:                // 0x3e
        case Opcodes::Z80_Ed_Nop_0xEd_0x3f:                // 0x3f

        case Opcodes::Z80_Ed_Nop_0xEd_0x77:
        case Opcodes::Z80_Ed_Nop_0xEd_0x7f:                // 0x7f
        case Opcodes::Z80_Ed_Nop_0xEd_0x80:                // 0x80
        case Opcodes::Z80_Ed_Nop_0xEd_0x81:                // 0x81
        case Opcodes::Z80_Ed_Nop_0xEd_0x82:                // 0x82
        case Opcodes::Z80_Ed_Nop_0xEd_0x83:                // 0x83
        case Opcodes::Z80_Ed_Nop_0xEd_0x84:                // 0x84
        case Opcodes::Z80_Ed_Nop_0xEd_0x85:                // 0x85
        case Opcodes::Z80_Ed_Nop_0xEd_0x86:                // 0x86
        case Opcodes::Z80_Ed_Nop_0xEd_0x87:                // 0x87
        case Opcodes::Z80_Ed_Nop_0xEd_0x88:                // 0x88
        case Opcodes::Z80_Ed_Nop_0xEd_0x89:                // 0x89
        case Opcodes::Z80_Ed_Nop_0xEd_0x8a:                // 0x8a
        case Opcodes::Z80_Ed_Nop_0xEd_0x8b:                // 0x8b
        case Opcodes::Z80_Ed_Nop_0xEd_0x8c:                // 0x8c
        case Opcodes::Z80_Ed_Nop_0xEd_0x8d:                // 0x8d
        case Opcodes::Z80_Ed_Nop_0xEd_0x8e:                // 0x8e
        case Opcodes::Z80_Ed_Nop_0xEd_0x8f:                // 0x8f
        case Opcodes::Z80_Ed_Nop_0xEd_0x90:                // 0x90
        case Opcodes::Z80_Ed_Nop_0xEd_0x91:                // 0x91
        case Opcodes::Z80_Ed_Nop_0xEd_0x92:                // 0x92
        case Opcodes::Z80_Ed_Nop_0xEd_0x93:                // 0x93
        case Opcodes::Z80_Ed_Nop_0xEd_0x94:                // 0x94
        case Opcodes::Z80_Ed_Nop_0xEd_0x95:                // 0x95
        case Opcodes::Z80_Ed_Nop_0xEd_0x96:                // 0x96
        case Opcodes::Z80_Ed_Nop_0xEd_0x97:                // 0x97
        case Opcodes::Z80_Ed_Nop_0xEd_0x98:                // 0x98
        case Opcodes::Z80_Ed_Nop_0xEd_0x99:                // 0x99
        case Opcodes::Z80_Ed_Nop_0xEd_0x9a:                // 0x9a
        case Opcodes::Z80_Ed_Nop_0xEd_0x9b:                // 0x9b
        case Opcodes::Z80_Ed_Nop_0xEd_0x9c:                // 0x9c
        case Opcodes::Z80_Ed_Nop_0xEd_0x9d:                // 0x9d
        case Opcodes::Z80_Ed_Nop_0xEd_0x9e:                // 0x9e
        case Opcodes::Z80_Ed_Nop_0xEd_0x9f:                // 0x9f

        case Opcodes::Z80_Ed_Nop_0xEd_0xA4:                // 0xa4
        case Opcodes::Z80_Ed_Nop_0xEd_0xA5:                // 0xa5
        case Opcodes::Z80_Ed_Nop_0xEd_0xA6:                // 0xa6
        case Opcodes::Z80_Ed_Nop_0xEd_0xA7:                // 0xa7
        
        case Opcodes::Z80_Ed_Nop_0xEd_0xAc:                // 0xac
        case Opcodes::Z80_Ed_Nop_0xEd_0xAd:                // 0xad
        case Opcodes::Z80_Ed_Nop_0xEd_0xAe:                // 0xae
        case Opcodes::Z80_Ed_Nop_0xEd_0xAf:                // 0xaf

        case Opcodes::Z80_Ed_Nop_0xEd_0xB4:                // 0xb4
        case Opcodes::Z80_Ed_Nop_0xEd_0xB5:                // 0xb5
        case Opcodes::Z80_Ed_Nop_0xEd_0xB6:                // 0xb6
        case Opcodes::Z80_Ed_Nop_0xEd_0xB7:                // 0xb7

        case Opcodes::Z80_Ed_Nop_0xEd_0xBc:                // 0xbc
        case Opcodes::Z80_Ed_Nop_0xEd_0xBd:                // 0xbd
        case Opcodes::Z80_Ed_Nop_0xEd_0xBe:                // 0xbe
        case Opcodes::Z80_Ed_Nop_0xEd_0xBf:                // 0xbf
        case Opcodes::Z80_Ed_Nop_0xEd_0xC0:                // 0xc0
        case Opcodes::Z80_Ed_Nop_0xEd_0xC1:                // 0xc1
        case Opcodes::Z80_Ed_Nop_0xEd_0xC2:                // 0xc2
        case Opcodes::Z80_Ed_Nop_0xEd_0xC3:                // 0xc3
        case Opcodes::Z80_Ed_Nop_0xEd_0xC4:                // 0xc4
        case Opcodes::Z80_Ed_Nop_0xEd_0xC5:                // 0xc5
        case Opcodes::Z80_Ed_Nop_0xEd_0xC6:                // 0xc6
        case Opcodes::Z80_Ed_Nop_0xEd_0xC7:                // 0xc7
        case Opcodes::Z80_Ed_Nop_0xEd_0xC8:                // 0xc8
        case Opcodes::Z80_Ed_Nop_0xEd_0xC9:                // 0xc9
        case Opcodes::Z80_Ed_Nop_0xEd_0xCa:                // 0xca
        case Opcodes::Z80_Ed_Nop_0xEd_0xCb:                // 0xcb
        case Opcodes::Z80_Ed_Nop_0xEd_0xCc:                // 0xcc
        case Opcodes::Z80_Ed_Nop_0xEd_0xCd:                // 0xcd
        case Opcodes::Z80_Ed_Nop_0xEd_0xCe:                // 0xce
        case Opcodes::Z80_Ed_Nop_0xEd_0xCf:                // 0xcf
        case Opcodes::Z80_Ed_Nop_0xEd_0xD0:                // 0xd0
        case Opcodes::Z80_Ed_Nop_0xEd_0xD1:                // 0xd1
        case Opcodes::Z80_Ed_Nop_0xEd_0xD2:                // 0xd2
        case Opcodes::Z80_Ed_Nop_0xEd_0xD3:                // 0xd3
        case Opcodes::Z80_Ed_Nop_0xEd_0xD4:                // 0xd4
        case Opcodes::Z80_Ed_Nop_0xEd_0xD5:                // 0xd5
        case Opcodes::Z80_Ed_Nop_0xEd_0xD6:                // 0xd6
        case Opcodes::Z80_Ed_Nop_0xEd_0xD7:                // 0xd7
        case Opcodes::Z80_Ed_Nop_0xEd_0xD8:                // 0xd8
        case Opcodes::Z80_Ed_Nop_0xEd_0xD9:                // 0xd9
        case Opcodes::Z80_Ed_Nop_0xEd_0xDa:                // 0xda
        case Opcodes::Z80_Ed_Nop_0xEd_0xDb:                // 0xdb
        case Opcodes::Z80_Ed_Nop_0xEd_0xDc:                // 0xdc
        case Opcodes::Z80_Ed_Nop_0xEd_0xDd:                // 0xdd
        case Opcodes::Z80_Ed_Nop_0xEd_0xDe:                // 0xde
        case Opcodes::Z80_Ed_Nop_0xEd_0xDf:                // 0xdf
        case Opcodes::Z80_Ed_Nop_0xEd_0xE0:                // 0xe0
        case Opcodes::Z80_Ed_Nop_0xEd_0xE1:                // 0xe1
        case Opcodes::Z80_Ed_Nop_0xEd_0xE2:                // 0xe2
        case Opcodes::Z80_Ed_Nop_0xEd_0xE3:                // 0xe3
        case Opcodes::Z80_Ed_Nop_0xEd_0xE4:                // 0xe4
        case Opcodes::Z80_Ed_Nop_0xEd_0xE5:                // 0xe5
        case Opcodes::Z80_Ed_Nop_0xEd_0xE6:                // 0xe6
        case Opcodes::Z80_Ed_Nop_0xEd_0xE7:                // 0xe7
        case Opcodes::Z80_Ed_Nop_0xEd_0xE8:                // 0xe8
        case Opcodes::Z80_Ed_Nop_0xEd_0xE9:                // 0xe9
        case Opcodes::Z80_Ed_Nop_0xEd_0xEa:                // 0xea
        case Opcodes::Z80_Ed_Nop_0xEd_0xEb:                // 0xeb
        case Opcodes::Z80_Ed_Nop_0xEd_0xEc:                // 0xec
        case Opcodes::Z80_Ed_Nop_0xEd_0xEd:                // 0xed
        case Opcodes::Z80_Ed_Nop_0xEd_0xEe:                // 0xee
        case Opcodes::Z80_Ed_Nop_0xEd_0xEf:                // 0xef
        case Opcodes::Z80_Ed_Nop_0xEd_0xF0:                // 0xf0
        case Opcodes::Z80_Ed_Nop_0xEd_0xF1:                // 0xf1
        case Opcodes::Z80_Ed_Nop_0xEd_0xF2:                // 0xf2
        case Opcodes::Z80_Ed_Nop_0xEd_0xF3:                // 0xf3
        case Opcodes::Z80_Ed_Nop_0xEd_0xF4:                // 0xf4
        case Opcodes::Z80_Ed_Nop_0xEd_0xF5:                // 0xf5
        case Opcodes::Z80_Ed_Nop_0xEd_0xF6:                // 0xf6
        case Opcodes::Z80_Ed_Nop_0xEd_0xF7:                // 0xf7
        case Opcodes::Z80_Ed_Nop_0xEd_0xF8:                // 0xf8
        case Opcodes::Z80_Ed_Nop_0xEd_0xF9:                // 0xf9
        case Opcodes::Z80_Ed_Nop_0xEd_0xFa:                // 0xfa
        case Opcodes::Z80_Ed_Nop_0xEd_0xFb:                // 0xfb
        case Opcodes::Z80_Ed_Nop_0xEd_0xFc:                // 0xfc
        case Opcodes::Z80_Ed_Nop_0xEd_0xFd:                // 0xfd
        case Opcodes::Z80_Ed_Nop_0xEd_0xFe:                // 0xfe
        case Opcodes::Z80_Ed_Nop_0xEd_0xFf:                // 0xff
            return {
                    .instruction = Instruction::NOP,
                    .operands = {},
                    .size = 2,
            };

        default:
            Util::debugln("disassembly of opcode 0xed {:#02x} not yet implemented", static_cast<std::uint16_t>(*machineCode));
            break;
    }

    return {
        .instruction = Instruction::NOP,
        .operands = {},
        .size = 1,
    };
}

Mnemonic Disassembler::disassembleOneDdOrFd(const Register16 reg, const UnsignedByte * machineCode)
{
    switch (*machineCode) {
        case Opcodes::Z80_DdOrFd_Inc_IndirectIxdOrIyd:                // 0x34
            return {
                .instruction = Instruction::INC,
                .operands = {
                    { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Dec_IndirectIxdOrIyd:                // 0x35
            return {
                .instruction = Instruction::DEC,
                .operands = {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_N:                // 0x36
            return {
                .instruction = Instruction::LD,
                .operands = {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                        { .mode = AddressingMode::Immediate, .unsignedByte = *(machineCode + 1), },
                },
                .size = 4,
            };

        case Opcodes::Z80_DdOrFd_Jr_C_d:                // 0x38
            return {
                .instruction = Instruction::JRC,
                .operands = {
                    { .mode = AddressingMode::Relative, .unsignedByte = *(machineCode + 1), },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_B_IndirectIxdOrIyd:                // 0x46
            return {
                .instruction = Instruction::LD,
                .operands = {
                        { .mode = AddressingMode::Register8, .register8 = Register8::B, },
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_C_IndirectIxdOrIyd:                // 0x4e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::C, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_D_IndirectIxdOrIyd:                // 0x56
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::D, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_E_IndirectIxdOrIyd:                // 0x5e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::E, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_H_IndirectIxdOrIyd:                // 0x66
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::H, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_L_IndirectIxdOrIyd:                // 0x6e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::L, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_B:                // 0x70
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::B, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_C:                // 0x71
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::C, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_D:                // 0x72
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::D, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_E:                // 0x73
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::E, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_H:                // 0x74
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::H, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_L:                // 0x75
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::L, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_IndirectIxdOrIyd_A:                // 0x77
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Ld_A_IndirectIxdOrIyd:                // 0x7e
            return {
                    .instruction = Instruction::LD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Add_A_IndirectIxdOrIyd:                // 0x86
            return {
                .instruction = Instruction::ADD,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Adc_A_IndirectIxdOrIyd:                // 0x8e
            return {
                    .instruction = Instruction::ADC,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Sub_IndirectIxdOrIyd:                // 0x96
            return {
                    .instruction = Instruction::SUB,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Sbc_A_IndirectIxdOrIyd:                // 0x9e
            return {
                    .instruction = Instruction::SBC,
                    .operands = {
                            { .mode = AddressingMode::Register8, .register8 = Register8::A, },
                            { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                    },
                    .size = 3,
            };

        case Opcodes::Z80_DdOrFd_And_IndirectIxdOrIyd:                // 0xa6
            return {
                .instruction = Instruction::AND,
                .operands = {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Xor_IndirectIxdOrIyd:                // 0xae
            return {
                .instruction = Instruction::XOR,
                .operands = {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Or_IndirectIxdOrIyd:                // 0xb6
            return {
                .instruction = Instruction::OR,
                .operands = {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Cp_IndirectIxdOrIyd:                // 0xbe
            return {
                .instruction = Instruction::CP,
                .operands = {
                        { .mode = AddressingMode::Indexed, .indexedAddress = { .register16 = reg, .offset = static_cast<SignedByte>(*(machineCode + 1)),}, },
                },
                .size = 3,
            };

        case Opcodes::Z80_DdOrFd_Prefix_Cb:                // 0xcb
            return disassembleOneDdCbOrFdCb(reg, machineCode + 1);

        // these are all expensive replicas of the plain instructions
        case Opcodes::Z80_DdOrFd_Nop:                // 0x00
        case Opcodes::Z80_DdOrFd_Ld_Bc_Nn:                // 0x01
        case Opcodes::Z80_DdOrFd_Ld_IndirectBc_A:                // 0x02
        case Opcodes::Z80_DdOrFd_Inc_Bc:                // 0x03
        case Opcodes::Z80_DdOrFd_Inc_B:                // 0x04
        case Opcodes::Z80_DdOrFd_Dec_B:                // 0x05
        case Opcodes::Z80_DdOrFd_Ld_B_N:                // 0x06
        case Opcodes::Z80_DdOrFd_Rlca:                // 0x07
        case Opcodes::Z80_DdOrFd_Ex_Af_AfShadow:                // 0x08
        case Opcodes::Z80_DdOrFd_Add_IxOrIy_Bc:                // 0x09
        case Opcodes::Z80_DdOrFd_Ld_A_IndirectBc:                // 0x0a
        case Opcodes::Z80_DdOrFd_Dec_Bc:                // 0x0b
        case Opcodes::Z80_DdOrFd_Inc_C:                // 0x0c
        case Opcodes::Z80_DdOrFd_Dec_C:                // 0x0d
        case Opcodes::Z80_DdOrFd_Ld_C_N:                // 0x0e
        case Opcodes::Z80_DdOrFd_Rrca:                // 0x0f
        case Opcodes::Z80_DdOrFd_Djnz_d:                // 0x10
        case Opcodes::Z80_DdOrFd_Ld_De_Nn:                // 0x11
        case Opcodes::Z80_DdOrFd_Ld_IndirectDe_A:                // 0x12
        case Opcodes::Z80_DdOrFd_Inc_De:                // 0x13
        case Opcodes::Z80_DdOrFd_Inc_D:                // 0x14
        case Opcodes::Z80_DdOrFd_Dec_D:                // 0x15
        case Opcodes::Z80_DdOrFd_Ld_D_N:                // 0x16
        case Opcodes::Z80_DdOrFd_Rla:                // 0x17
        case Opcodes::Z80_DdOrFd_Jr_d:                // 0x18
        case Opcodes::Z80_DdOrFd_Add_IxOrIy_De:                // 0x19
        case Opcodes::Z80_DdOrFd_Ld_A_IndirectDe:                // 0x1a
        case Opcodes::Z80_DdOrFd_Dec_De:                // 0x1b
        case Opcodes::Z80_DdOrFd_Inc_E:                // 0x1c
        case Opcodes::Z80_DdOrFd_Dec_E:                // 0x1d
        case Opcodes::Z80_DdOrFd_Ld_E_N:                // 0x1e
        case Opcodes::Z80_DdOrFd_Rra:                // 0x1f
        case Opcodes::Z80_DdOrFd_Jr_Nz_d:                // 0x20
        case Opcodes::Z80_DdOrFd_Ld_IxOrIy_Nn:                // 0x21
        case Opcodes::Z80_DdOrFd_Ld_IndirectNn_IxOrIy:                // 0x22
        case Opcodes::Z80_DdOrFd_Inc_IxOrIy:                // 0x23
        case Opcodes::Z80_DdOrFd_Inc_IxhOrIyh:                // 0x24
        case Opcodes::Z80_DdOrFd_Dec_IxhOrIyh:                // 0x25
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_N:                // 0x26
        case Opcodes::Z80_DdOrFd_Daa:                // 0x27
        case Opcodes::Z80_DdOrFd_Jr_Z_d:                // 0x28
        case Opcodes::Z80_DdOrFd_Add_IxOrIy_IxOrIy:                // 0x29
        case Opcodes::Z80_DdOrFd_Ld_IxOrIy_IndirectNn:                // 0x2a
        case Opcodes::Z80_DdOrFd_Dec_IxOrIy:                // 0x2b
        case Opcodes::Z80_DdOrFd_Inc_IxlOrIyl:                // 0x2c
        case Opcodes::Z80_DdOrFd_Dec_IxlOrIyl:                // 0x2d
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_N:                // 0x2e
        case Opcodes::Z80_DdOrFd_Cpl:                // 0x2f
        case Opcodes::Z80_DdOrFd_Jr_Nc_d:                // 0x30
        case Opcodes::Z80_DdOrFd_Ld_Sp_Nn:                // 0x31
        case Opcodes::Z80_DdOrFd_Ld_IndirectNn_A:                // 0x32
        case Opcodes::Z80_DdOrFd_Inc_Sp:                // 0x33
        case Opcodes::Z80_DdOrFd_Scf:                // 0x37
        case Opcodes::Z80_DdOrFd_Add_IxOrIy_Sp:                // 0x39
        case Opcodes::Z80_DdOrFd_Ld_A_IndirectNn:                // 0x3a
        case Opcodes::Z80_DdOrFd_Dec_Sp:                // 0x3b
        case Opcodes::Z80_DdOrFd_Inc_A:                // 0x3c
        case Opcodes::Z80_DdOrFd_Dec_A:                // 0x3d
        case Opcodes::Z80_DdOrFd_Ld_A_N:                // 0x3e
        case Opcodes::Z80_DdOrFd_Ccf:                // 0x3f
        case Opcodes::Z80_DdOrFd_Ld_B_B:                // 0x40
        case Opcodes::Z80_DdOrFd_Ld_B_C:                // 0x41
        case Opcodes::Z80_DdOrFd_Ld_B_D:                // 0x42
        case Opcodes::Z80_DdOrFd_Ld_B_E:                // 0x43
        case Opcodes::Z80_DdOrFd_Ld_B_IxhOrIyh:                // 0x44
        case Opcodes::Z80_DdOrFd_Ld_B_IxlOrIyl:                // 0x45
        case Opcodes::Z80_DdOrFd_Ld_B_A:                // 0x47
        case Opcodes::Z80_DdOrFd_Ld_C_B:                // 0x48
        case Opcodes::Z80_DdOrFd_Ld_C_C:                // 0x49
        case Opcodes::Z80_DdOrFd_Ld_C_D:                // 0x4a
        case Opcodes::Z80_DdOrFd_Ld_C_E:                // 0x4b
        case Opcodes::Z80_DdOrFd_Ld_C_IxhOrIyh:                // 0x4c
        case Opcodes::Z80_DdOrFd_Ld_C_IxlOrIyl:                // 0x4d
        case Opcodes::Z80_DdOrFd_Ld_C_A:                // 0x4f
        case Opcodes::Z80_DdOrFd_Ld_D_B:                // 0x50
        case Opcodes::Z80_DdOrFd_Ld_D_C:                // 0x51
        case Opcodes::Z80_DdOrFd_Ld_D_D:                // 0x52
        case Opcodes::Z80_DdOrFd_Ld_D_E:                // 0x53
        case Opcodes::Z80_DdOrFd_Ld_D_IxhOrIyh:                // 0x54
        case Opcodes::Z80_DdOrFd_Ld_D_IxlOrIyl:                // 0x55
        case Opcodes::Z80_DdOrFd_Ld_D_A:                // 0x57
        case Opcodes::Z80_DdOrFd_Ld_E_B:                // 0x58
        case Opcodes::Z80_DdOrFd_Ld_E_C:                // 0x59
        case Opcodes::Z80_DdOrFd_Ld_E_D:                // 0x5a
        case Opcodes::Z80_DdOrFd_Ld_E_E:                // 0x5b
        case Opcodes::Z80_DdOrFd_Ld_E_IxhOrIyh:                // 0x5c
        case Opcodes::Z80_DdOrFd_Ld_E_IxlOrIyl:                // 0x5d
        case Opcodes::Z80_DdOrFd_Ld_E_A:                // 0x5f
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_B:                // 0x60
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_C:                // 0x61
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_D:                // 0x62
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_E:                // 0x63
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_IxhOrIyh:                // 0x64
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_IxlOrIyl:                // 0x65
        case Opcodes::Z80_DdOrFd_Ld_IxhOrIyh_A:                // 0x67
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_B:                // 0x68
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_C:                // 0x69
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_D:                // 0x6a
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_E:                // 0x6b
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_IxhOrIyh:                // 0x6c
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_IxlOrIyl:                // 0x6d
        case Opcodes::Z80_DdOrFd_Ld_IxlOrIyl_A:                // 0x6f
        case Opcodes::Z80_DdOrFd_Halt:                // 0x76
        case Opcodes::Z80_DdOrFd_Ld_A_B:                // 0x78
        case Opcodes::Z80_DdOrFd_Ld_A_C:                // 0x79
        case Opcodes::Z80_DdOrFd_Ld_A_D:                // 0x7a
        case Opcodes::Z80_DdOrFd_Ld_A_E:                // 0x7b
        case Opcodes::Z80_DdOrFd_Ld_A_IxhOrIyh:                // 0x7c
        case Opcodes::Z80_DdOrFd_Ld_A_IxlOrIyl:                // 0x7d
        case Opcodes::Z80_DdOrFd_Ld_A_A:                // 0x7f
        case Opcodes::Z80_DdOrFd_Add_A_B:                // 0x80
        case Opcodes::Z80_DdOrFd_Add_A_C:                // 0x81
        case Opcodes::Z80_DdOrFd_Add_A_D:                // 0x82
        case Opcodes::Z80_DdOrFd_Add_A_E:                // 0x83
        case Opcodes::Z80_DdOrFd_Add_A_IxhOrIyh:                // 0x84
        case Opcodes::Z80_DdOrFd_Add_A_IxlOrIyl:                // 0x85
        case Opcodes::Z80_DdOrFd_Add_A_A:                // 0x87
        case Opcodes::Z80_DdOrFd_Adc_A_B:                // 0x88
        case Opcodes::Z80_DdOrFd_Adc_A_C:                // 0x89
        case Opcodes::Z80_DdOrFd_Adc_A_D:                // 0x8a
        case Opcodes::Z80_DdOrFd_Adc_A_E:                // 0x8b
        case Opcodes::Z80_DdOrFd_Adc_A_IxhOrIyh:                // 0x8c
        case Opcodes::Z80_DdOrFd_Adc_A_IxlOrIyl:                // 0x8d
        case Opcodes::Z80_DdOrFd_Adc_A_A:                // 0x8f
        case Opcodes::Z80_DdOrFd_Sub_B:                // 0x90
        case Opcodes::Z80_DdOrFd_Sub_C:                // 0x91
        case Opcodes::Z80_DdOrFd_Sub_D:                // 0x92
        case Opcodes::Z80_DdOrFd_Sub_E:                // 0x93
        case Opcodes::Z80_DdOrFd_Sub_IxhOrIyh:                // 0x94
        case Opcodes::Z80_DdOrFd_Sub_IxlOrIyl:                // 0x95
        case Opcodes::Z80_DdOrFd_Sub_A:                // 0x97
        case Opcodes::Z80_DdOrFd_Sbc_A_B:                // 0x98
        case Opcodes::Z80_DdOrFd_Sbc_A_C:                // 0x99
        case Opcodes::Z80_DdOrFd_Sbc_A_D:                // 0x9a
        case Opcodes::Z80_DdOrFd_Sbc_A_E:                // 0x9b
        case Opcodes::Z80_DdOrFd_Sbc_A_IxhOrIyh:                // 0x9c
        case Opcodes::Z80_DdOrFd_Sbc_A_IxlOrIyl:                // 0x9d
        case Opcodes::Z80_DdOrFd_Sbc_A_A:                // 0x9f
        case Opcodes::Z80_DdOrFd_And_B:                // 0xa0
        case Opcodes::Z80_DdOrFd_And_C:                // 0xa1
        case Opcodes::Z80_DdOrFd_And_D:                // 0xa2
        case Opcodes::Z80_DdOrFd_And_E:                // 0xa3
        case Opcodes::Z80_DdOrFd_And_IxhOrIyh:                // 0xa4
        case Opcodes::Z80_DdOrFd_And_IxlOrIyl:                // 0xa5
        case Opcodes::Z80_DdOrFd_And_A:                // 0xa7
        case Opcodes::Z80_DdOrFd_Xor_B:                // 0xa8
        case Opcodes::Z80_DdOrFd_Xor_C:                // 0xa9
        case Opcodes::Z80_DdOrFd_Xor_D:                // 0xaa
        case Opcodes::Z80_DdOrFd_Xor_E:                // 0xab
        case Opcodes::Z80_DdOrFd_Xor_IxhOrIyh:                // 0xac
        case Opcodes::Z80_DdOrFd_Xor_IxlOrIyl:                // 0xad
        case Opcodes::Z80_DdOrFd_Xor_A:                // 0xaf
        case Opcodes::Z80_DdOrFd_Or_B:                // 0xb0
        case Opcodes::Z80_DdOrFd_Or_C:                // 0xb1
        case Opcodes::Z80_DdOrFd_Or_D:                // 0xb2
        case Opcodes::Z80_DdOrFd_Or_E:                // 0xb3
        case Opcodes::Z80_DdOrFd_Or_IxhOrIyh:                // 0xb4
        case Opcodes::Z80_DdOrFd_Or_IxlOrIyl:                // 0xb5
        case Opcodes::Z80_DdOrFd_Or_A:                // 0xb7
        case Opcodes::Z80_DdOrFd_Cp_B:                // 0xb8
        case Opcodes::Z80_DdOrFd_Cp_C:                // 0xb9
        case Opcodes::Z80_DdOrFd_Cp_D:                // 0xba
        case Opcodes::Z80_DdOrFd_Cp_E:                // 0xbb
        case Opcodes::Z80_DdOrFd_Cp_IxhOrIyh:                // 0xbc
        case Opcodes::Z80_DdOrFd_Cp_IxlOrIyl:                // 0xbd
        case Opcodes::Z80_DdOrFd_Cp_A:                // 0xbf
        case Opcodes::Z80_DdOrFd_Ret_Nz:                // 0xc0
        case Opcodes::Z80_DdOrFd_Pop_Bc:                // 0xc1
        case Opcodes::Z80_DdOrFd_Jp_Nz_Nn:                // 0xc2
        case Opcodes::Z80_DdOrFd_Jp_Nn:                // 0xc3
        case Opcodes::Z80_DdOrFd_Call_Nz_Nn:                // 0xc4
        case Opcodes::Z80_DdOrFd_Push_Bc:                // 0xc5
        case Opcodes::Z80_DdOrFd_Add_A_N:                // 0xc6
        case Opcodes::Z80_DdOrFd_Rst_00:                // 0xc7
        case Opcodes::Z80_DdOrFd_Ret_Z:                // 0xc8
        case Opcodes::Z80_DdOrFd_Ret:                // 0xc9
        case Opcodes::Z80_DdOrFd_Jp_Z_Nn:                // 0xca
        case Opcodes::Z80_DdOrFd_Call_Z_Nn:                // 0xcc
        case Opcodes::Z80_DdOrFd_Call_Nn:                // 0xcd
        case Opcodes::Z80_DdOrFd_Adc_A_N:                // 0xce
        case Opcodes::Z80_DdOrFd_Rst_08:                // 0xcf
        case Opcodes::Z80_DdOrFd_Ret_Nc:                // 0xd0
        case Opcodes::Z80_DdOrFd_Pop_De:                // 0xd1
        case Opcodes::Z80_DdOrFd_Jp_Nc_Nn:                // 0xd2
        case Opcodes::Z80_DdOrFd_Out_IndirectN_A:                // 0xd3
        case Opcodes::Z80_DdOrFd_Call_Nc_Nn:                // 0xd4
        case Opcodes::Z80_DdOrFd_Push_De:                // 0xd5
        case Opcodes::Z80_DdOrFd_Sub_N:                // 0xd6
        case Opcodes::Z80_DdOrFd_Rst_10:                // 0xd7
        case Opcodes::Z80_DdOrFd_Ret_C:                // 0xd8
        case Opcodes::Z80_DdOrFd_Exx:                // 0xd9
        case Opcodes::Z80_DdOrFd_Jp_C_Nn:                // 0xda
        case Opcodes::Z80_DdOrFd_In_A_IndirectN:                // 0xdb
        case Opcodes::Z80_DdOrFd_Call_C_Nn:                // 0xdc
        case Opcodes::Z80_DdOrFd_Sbc_A_N:                // 0xde
        case Opcodes::Z80_DdOrFd_Rst_18:                // 0xdf
        case Opcodes::Z80_DdOrFd_Ret_Po:                // 0xe0
        case Opcodes::Z80_DdOrFd_Pop_IxOrIy:                // 0xe1
        case Opcodes::Z80_DdOrFd_Jp_Po_Nn:                // 0xe2
        case Opcodes::Z80_DdOrFd_Ex_IndirectSp_IxOrIy:                // 0xe3
        case Opcodes::Z80_DdOrFd_Call_Po_Nn:                // 0xe4
        case Opcodes::Z80_DdOrFd_Push_IxOrIy:                // 0xe5
        case Opcodes::Z80_DdOrFd_And_N:                // 0xe6
        case Opcodes::Z80_DdOrFd_Rst_20:                // 0xe7
        case Opcodes::Z80_DdOrFd_Ret_Pe:                // 0xe8
        case Opcodes::Z80_DdOrFd_Jp_IxOrIy:                // 0xe9
        case Opcodes::Z80_DdOrFd_Jp_Pe_Nn:                // 0xea
        case Opcodes::Z80_DdOrFd_Ex_De_Hl:                // 0xeb
        case Opcodes::Z80_DdOrFd_Call_Pe_Nn:                // 0xec
        case Opcodes::Z80_DdOrFd_Prefix_Ed:                // 0xed
        case Opcodes::Z80_DdOrFd_Xor_N:                // 0xee
        case Opcodes::Z80_DdOrFd_Rst_28:                // 0xef
        case Opcodes::Z80_DdOrFd_Ret_P:                // 0xf0
        case Opcodes::Z80_DdOrFd_Pop_Af:                // 0xf1
        case Opcodes::Z80_DdOrFd_Jp_P_Nn:                // 0xf2
        case Opcodes::Z80_DdOrFd_Di:                // 0xf3
        case Opcodes::Z80_DdOrFd_Call_P_Nn:                // 0xf4
        case Opcodes::Z80_DdOrFd_Push_Af:                // 0xf5
        case Opcodes::Z80_DdOrFd_Or_N:                // 0xf6
        case Opcodes::Z80_DdOrFd_Rst_30:                // 0xf7
        case Opcodes::Z80_DdOrFd_Ret_M:                // 0xf8
        case Opcodes::Z80_DdOrFd_Ld_Sp_IxOrIy:                // 0xf9
        case Opcodes::Z80_DdOrFd_Jp_M_Nn:                // 0xfa
        case Opcodes::Z80_DdOrFd_Ei:                // 0xfb
        case Opcodes::Z80_DdOrFd_Call_M_Nn:                // 0xfc
        case Opcodes::Z80_DdOrFd_Cp_N:                // 0xfe
        case Opcodes::Z80_DdOrFd_Rst_38:                // 0xff
        {
            auto mnemonic = disassembleOnePlain(machineCode);
            ++mnemonic.size;
            return mnemonic;
        }

        case Opcodes::Z80_DdOrFd_Prefix_Dd:                // 0xdd
        case Opcodes::Z80_DdOrFd_Prefix_Fd:                // 0xfd
        {
            // TODO this is not strictly correct - sequences of 0xdd/0xfd result in an IX/IY instruction based on the
            //  byte following the last 0xdd/0xfd in the sequence.
            return {
                .instruction = Instruction::NOP,
                .operands = {},
                .size = 2,
            };
        }

        default:
            Util::debugln("disassembly of opcode {} {:#02x} not yet implemented",
                Register16::IX == reg ? "0xdd" : "0xfd",
                static_cast<std::uint16_t>(*machineCode)
            );
            break;
    }

    return {
        .instruction = Instruction::NOP,
        .operands = {},
        .size = 1,
    };
}

Mnemonic Disassembler::disassembleOneDdCbOrFdCb(const Register16 reg, const ::Z80::UnsignedByte * machineCode)
{
    static constexpr int OpcodeSize = 4;

    // NOTE these opcodes are of the form 0xdd 0xcb DD II or 0xfd 0xcb DD II where II is the 8-bit opcode and DD is the
    // 8-bit 2s-complement offset to use with IX or IY
    const auto opcode = *(machineCode + 1);
    const auto offset = *(machineCode);
    
    switch (opcode) {
        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_B:                       // 0x00
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_C:                       // 0x01
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_D:                       // 0x02
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_E:                       // 0x03
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_H:                       // 0x04
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_L:                       // 0x05
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd:                          // 0x06
            return {
                .instruction = Instruction::RLC,
                .operands = {
                        {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rlc_IndirectIxdOrIyd_A:                       // 0x07
            return {
                .instruction = Instruction::RLC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_B:                       // 0x08
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_C:                       // 0x09
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_D:                       // 0x0a
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_E:                       // 0x0b
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_H:                       // 0x0c
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_L:                       // 0x0d
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd:                          // 0x0e
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rrc_IndirectIxdOrIyd_A:                       // 0x0f
            return {
                .instruction = Instruction::RRC,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };


        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_B:                        // 0x10
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_C:                        // 0x11
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_D:                        // 0x12
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_E:                        // 0x13
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_H:                        // 0x14
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_L:                        // 0x15
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd:                           // 0x16
            return {
                .instruction = Instruction::RL,
                .operands = {
                        {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rl_IndirectIxdOrIyd_A:                        // 0x17
            return {
                .instruction = Instruction::RL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };


        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_B:                        // 0x18
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_C:                        // 0x19
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_D:                        // 0x1a
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_E:                        // 0x1b
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_H:                        // 0x1c
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_L:                        // 0x1d
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd:                          // 0x1e
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Rr_IndirectIxdOrIyd_A:                        // 0x1f
            return {
                .instruction = Instruction::RR,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };


        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_B:                       // 0x20
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_C:                       // 0x21
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_D:                       // 0x22
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_E:                       // 0x23
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_H:                       // 0x24
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_L:                       // 0x25
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd:                          // 0x26
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sla_IndirectIxdOrIyd_A:                       // 0x27
            return {
                .instruction = Instruction::SLA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };


        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_B:                       // 0x28
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_C:                       // 0x29
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_D:                       // 0x2a
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_E:                       // 0x2b
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_H:                       // 0x2c
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_L:                       // 0x2d
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd:                          // 0x2e
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sra_IndirectIxdOrIyd_A:                       // 0x2f
            return {
                .instruction = Instruction::SRA,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };


        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_B:                       // 0x30
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_C:                       // 0x31
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_D:                       // 0x32
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_E:                       // 0x33
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_H:                       // 0x34
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_L:                       // 0x35
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd:                          // 0x36
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Sll_IndirectIxdOrIyd_A:                       // 0x37
            return {
                .instruction = Instruction::SLL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_B:                       // 0x38
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_C:                       // 0x39
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_D:                       // 0x3a
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_E:                       // 0x3b
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_H:                       // 0x3c
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_L:                       // 0x3d
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd:                          // 0x3e
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Srl_IndirectIxdOrIyd_A:                       // 0x3f
            return {
                .instruction = Instruction::SRL,
                .operands = {
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_B:                    // 0x40
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_C:                    // 0x41
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_D:                    // 0x42
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_E:                    // 0x43
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_H:                    // 0x44
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_L:                    // 0x45
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd:                          // 0x06
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_0_IndirectIxdOrIyd_A:                    // 0x47
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_B:                    // 0x48
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_C:                    // 0x49
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_D:                    // 0x4a
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_E:                    // 0x4b
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_H:                    // 0x4c
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_L:                    // 0x4d
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd:                          // 0x4e
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_1_IndirectIxdOrIyd_A:                    // 0x4f
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_B:                    // 0x50
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_C:                    // 0x51
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_D:                    // 0x52
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_E:                    // 0x53
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_H:                    // 0x54
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_L:                    // 0x55
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd:                          // 0x56
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_2_IndirectIxdOrIyd_A:                    // 0x57
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_B:                    // 0x58
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_C:                    // 0x59
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_D:                    // 0x5a
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_E:                    // 0x5b
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_H:                    // 0x5c
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_L:                    // 0x5d
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd:                          // 0x5e
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_3_IndirectIxdOrIyd_A:                    // 0x5f
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = OpcodeSize },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = 3,
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_B:                    // 0x60
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_C:                    // 0x61
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_D:                    // 0x62
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_E:                    // 0x63
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_H:                    // 0x64
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_L:                    // 0x65
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd:                          // 0x66
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_4_IndirectIxdOrIyd_A:                    // 0x67
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_B:                    // 0x68
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_C:                    // 0x69
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_D:                    // 0x6a
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_E:                    // 0x6b
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_H:                    // 0x6c
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_L:                    // 0x6d
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd:                          // 0x6e
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_5_IndirectIxdOrIyd_A:                    // 0x6f
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_B:                    // 0x70
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_C:                    // 0x71
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_D:                    // 0x72
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_E:                    // 0x73
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_H:                    // 0x74
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_L:                    // 0x75
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd:                          // 0x76
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_6_IndirectIxdOrIyd_A:                    // 0x77
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_B:                    // 0x78
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_C:                    // 0x79
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_D:                    // 0x7a
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_E:                    // 0x7b
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_H:                    // 0x7c
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_L:                    // 0x7d
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd:                          // 0x7e
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Bit_7_IndirectIxdOrIyd_A:                    // 0x7f
            return {
                .instruction = Instruction::BIT,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_B:                    // 0x80
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_C:                    // 0x81
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_D:                    // 0x82
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_E:                    // 0x83
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_H:                    // 0x84
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_L:                    // 0x85
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd:                          // 0x86
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_0_IndirectIxdOrIyd_A:                    // 0x87
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_B:                    // 0x88
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_C:                    // 0x89
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_D:                    // 0x8a
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_E:                    // 0x8b
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_H:                    // 0x8c
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_L:                    // 0x8d
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd:                          // 0x8e
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_1_IndirectIxdOrIyd_A:                    // 0x8f
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_B:                    // 0x90
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_C:                    // 0x91
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_D:                    // 0x92
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_E:                    // 0x93
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_H:                    // 0x94
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_L:                    // 0x95
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd:                          // 0x96
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_2_IndirectIxdOrIyd_A:                    // 0x97
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_B:                    // 0x98
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_C:                    // 0x99
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_D:                    // 0x9a
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_E:                    // 0x9b
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_H:                    // 0x9c
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_L:                    // 0x9d
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd:                          // 0x9e
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_3_IndirectIxdOrIyd_A:                    // 0x9f
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_B:                    // 0xa0
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_C:                    // 0xa1
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_D:                    // 0xa2
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_E:                    // 0xa3
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_H:                    // 0xa4
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_L:                    // 0xa5
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd:                          // 0xa6
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_4_IndirectIxdOrIyd_A:                    // 0xa7
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_B:                    // 0xa8
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_C:                    // 0xa9
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_D:                    // 0xaa
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_E:                    // 0xab
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_H:                    // 0xac
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_L:                    // 0xad
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd:                          // 0xae
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_5_IndirectIxdOrIyd_A:                    // 0xaf
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_B:                    // 0xb0
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_C:                    // 0xb1
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_D:                    // 0xb2
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_E:                    // 0xb3
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_H:                    // 0xb4
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_L:                    // 0xb5
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd:                          // 0xb6
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_6_IndirectIxdOrIyd_A:                    // 0xb7
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_B:                    // 0xb8
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_C:                    // 0xb9
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_D:                    // 0xba
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_E:                    // 0xbb
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_H:                    // 0xbc
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_L:                    // 0xbd
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd:                          // 0xbe
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Res_7_IndirectIxdOrIyd_A:                    // 0xbf
            return {
                .instruction = Instruction::RES,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_B:                    // 0xc0
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_C:                    // 0xc1
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_D:                    // 0xc2
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_E:                    // 0xc3
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_H:                    // 0xc4
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_L:                    // 0xc5
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd:                          // 0xc6
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_0_IndirectIxdOrIyd_A:                    // 0xc7
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 0, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_B:                    // 0xc8
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_C:                    // 0xc9
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_D:                    // 0xca
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_E:                    // 0xcb
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_H:                    // 0xcc
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_L:                    // 0xcd
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd:                          // 0xce
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_1_IndirectIxdOrIyd_A:                    // 0xcf
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 1, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_B:                    // 0xd0
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_C:                    // 0xd1
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_D:                    // 0xd2
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_E:                    // 0xd3
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_H:                    // 0xd4
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_L:                    // 0xd5
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd:                          // 0xd6
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_2_IndirectIxdOrIyd_A:                    // 0xd7
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 2, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_B:                    // 0xd8
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_C:                    // 0xd9
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_D:                    // 0xda
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_E:                    // 0xdb
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_H:                    // 0xdc
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_L:                    // 0xdd
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd:                          // 0xde
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_3_IndirectIxdOrIyd_A:                    // 0xdf
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 3, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_B:                    // 0xe0
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_C:                    // 0xe1
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_D:                    // 0xe2
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_E:                    // 0xe3
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_H:                    // 0xe4
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_L:                    // 0xe5
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd:                          // 0xe6
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_4_IndirectIxdOrIyd_A:                    // 0xe7
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 4, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_B:                    // 0xe8
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_C:                    // 0xe9
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_D:                    // 0xea
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_E:                    // 0xeb
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_H:                    // 0xec
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_L:                    // 0xed
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd:                          // 0xee
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_5_IndirectIxdOrIyd_A:                    // 0xef
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 5, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_B:                    // 0xf0
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_C:                    // 0xf1
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_D:                    // 0xf2
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_E:                    // 0xf3
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_H:                    // 0xf4
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_L:                    // 0xf5
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd:                          // 0xf6
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_6_IndirectIxdOrIyd_A:                    // 0xf7
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 6, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };


        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_B:                    // 0xf8
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::B,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_C:                    // 0xf9
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::C,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_D:                    // 0xfa
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::D,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_E:                    // 0xfb
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::E,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_H:                    // 0xfc
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::H,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_L:                    // 0xfd
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::L,},
                },
                .size = OpcodeSize
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd:                          // 0x06
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                },
                .size = OpcodeSize,
            };

        case Opcodes::Z80_DdOrFd_Cb_Set_7_IndirectIxdOrIyd_A:                    // 0xff
            return {
                .instruction = Instruction::SET,
                .operands = {
                    {.mode = AddressingMode::Bit, .unsignedByte = 7, },
                    {.mode = AddressingMode::Indexed, .indexedAddress = {.register16 = reg, .offset = static_cast<SignedByte>(offset), }, },
                    {.mode = AddressingMode::Register8, .register8 = Register8::A,},
                },
                .size = OpcodeSize
            };
        default:
            Util::debugln(
                "disassembly of opcode {} 0xcb {:#02x} not yet implemented",
                Register16::IX == reg ? "0xdd " : "0xfd ",
                static_cast<std::uint16_t>(*machineCode)
            );
            break;
    }

    return {
        .instruction = Instruction::NOP,
        .operands = {},
        .size = 1,
    };
}
