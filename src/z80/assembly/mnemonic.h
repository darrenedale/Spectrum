//
// Created by darren on 17/03/2021.
//

#ifndef SPECTRUM_MEMONIC_H
#define SPECTRUM_MEMONIC_H

#include <vector>
#include <string>
#include "operand.h"
#include "../types.h"

namespace Z80::Assembly
{
    enum class Instruction
    {
        LD, PUSH, POP, EX, EXX, LDI, LDIR, LDD, LDDR, CPI, CPIR, CPD, CPDR,
        ADD, ADC, SUB, SBC, AND, OR, XOR, CP, INC, DEC,
        DAA, CPL, NEG, CCF, SCF, NOP, HALT, DI, EI, IM0, IM1, IM2,
        RLCA, RLA, RRCA, RRA, RLC, RL, RRC, RR, SLA, SLL, SRA, SRL, RLD, RRD,
        BIT, SET, RES,
        JP, JPNZ, JPZ, JPNC, JPC, JPPO, JPPE, JPP, JPM,
        JR, JRNZ, JRZ, JRNC, JRC,
        DJNZ,
        CALL, CALLNZ, CALLZ, CALLNC, CALLC, CALLPO, CALLPE, CALLP, CALLM, CALLI,
        RET, RETNZ, RETZ, RETNC, RETC, RETPO, RETPE, RETP, RETM, RETI, RETN, RST,
        IN, INI, INIR, IND, INDR, OUT, OUTI, OTIR, OUTD, OTDR,
    };

    using Operands = std::vector<Operand>;

    struct Mnemonic
    {
        Instruction instruction;
        std::vector<Operand> operands;
        UnsignedByte size = 1;                  // size in bytes of the instruction
    };
}

namespace std
{
    std::string to_string(const Z80::Assembly::Instruction &);
    std::string to_string(const Z80::Assembly::Mnemonic &);
}

#endif //SPECTRUM_MEMONIC_H
