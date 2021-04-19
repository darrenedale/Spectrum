//
// Created by darren on 17/03/2021.
//

#include <sstream>
#include "mnemonic.h"
#include "../../util/debug.h"


using namespace Z80::Assembly;

std::string std::to_string(const Instruction & instruction)
{
    switch (instruction) {
        case Instruction::LD:
            return "LD";

        case Instruction::PUSH:
            return "PUSH";

        case Instruction::POP:
            return "POP";

        case Instruction::EX:
            return "EX";

        case Instruction::EXX:
            return "EXX";

        case Instruction::LDI:
            return "LDI";

        case Instruction::LDIR:
            return "LDIR";

        case Instruction::LDD:
            return "LDD";

        case Instruction::LDDR:
            return "LDDR";

        case Instruction::CPI:
            return "CPI";

        case Instruction::CPIR:
            return "CPIR";

        case Instruction::CPD:
            return "CPD";

        case Instruction::CPDR:
            return "CPDR";

        case Instruction::ADD:
            return "ADD";

        case Instruction::ADC:
            return "ADC";

        case Instruction::SUB:
            return "SUB";

        case Instruction::SBC:
            return "SBC";

        case Instruction::AND:
            return "AND";

        case Instruction::OR:
            return "OR";

        case Instruction::XOR:
            return "XOR";

        case Instruction::CP:
            return "CP";

        case Instruction::INC:
            return "INC";

        case Instruction::DEC:
            return "DEC";

        case Instruction::DAA:
            return "DAA";

        case Instruction::CPL:
            return "CPL";

        case Instruction::NEG:
            return "NEG";

        case Instruction::CCF:
            return "CCF";

        case Instruction::SCF:
            return "SCF";

        case Instruction::NOP:
            return "NOP";

        case Instruction::HALT:
            return "HALT";

        case Instruction::DI:
            return "DI";

        case Instruction::EI:
            return "EI";

        case Instruction::IM0:
            return "IM 0";

        case Instruction::IM1:
            return "IM 1";

        case Instruction::IM2:
            return "IM 2";

        case Instruction::RLCA:
            return "RLCA";

        case Instruction::RLA:
            return "RLA";

        case Instruction::RRCA:
            return "RRCA";

        case Instruction::RRA:
            return "RRA";

        case Instruction::RLC:
            return "RLC";

        case Instruction::RL:
            return "RL";

        case Instruction::RRC:
            return "RRC";

        case Instruction::RR:
            return "RR";

        case Instruction::SLA:
            return "SLA";

        case Instruction::SLL:
            return "SLL";

        case Instruction::SRA:
            return "SRA";

        case Instruction::SRL:
            return "SRL";

        case Instruction::RLD:
            return "RLD";

        case Instruction::RRD:
            return "RRD";

        case Instruction::BIT:
            return "BIT";

        case Instruction::SET:
            return "SET";

        case Instruction::RES:
            return "RES";

        case Instruction::JP:
            return "JP";

        case Instruction::JPNZ:
            return "JP NZ";

        case Instruction::JPZ:
            return "JP Z";

        case Instruction::JPNC:
            return "JP NC";

        case Instruction::JPC:
            return "JP C";

        case Instruction::JPPO:
            return "JP PO";

        case Instruction::JPPE:
            return "JP PE";

        case Instruction::JPP:
            return "JP P";

        case Instruction::JPM:
            return "JP M";

        case Instruction::JR:
            return "JR";

        case Instruction::JRNZ:
            return "JR NZ";

        case Instruction::JRZ:
            return "JR Z";

        case Instruction::JRNC:
            return "JR NC";

        case Instruction::JRC:
            return "JR C";

        case Instruction::DJNZ:
            return "DJNZ";

        case Instruction::CALL:
            return "CALL";

        case Instruction::CALLNZ:
            return "CALL NZ";

        case Instruction::CALLZ:
            return "CALL Z";

        case Instruction::CALLNC:
            return "CALL NC";

        case Instruction::CALLC:
            return "CALL C";

        case Instruction::CALLPE:
            return "CALL PE";

        case Instruction::CALLPO:
            return "CALL PO";

        case Instruction::CALLP:
            return "CALL P";

        case Instruction::CALLM:
            return "CALL M";

        case Instruction::RET:
            return "RET";

        case Instruction::RETNZ:
            return "RET NZ";

        case Instruction::RETZ:
            return "RET Z";

        case Instruction::RETNC:
            return "RET NC";

        case Instruction::RETC:
            return "RET C";

        case Instruction::RETPO:
            return "RET PO";

        case Instruction::RETPE:
            return "RET PE";

        case Instruction::RETP:
            return "RET P";

        case Instruction::RETM:
            return "RET M";

        case Instruction::RETI:
            return "RETI";

        case Instruction::RETN:
            return "RETN";

        case Instruction::RST:
            return "RST";

        case Instruction::IN:
            return "IN";

        case Instruction::INI:
            return "INI";

        case Instruction::INIR:
            return "INIR";

        case Instruction::IND:
            return "IND";

        case Instruction::INDR:
            return "INDR";

        case Instruction::OUT:
            return "OUT";

        case Instruction::OUTI:
            return "OUTI";

        case Instruction::OTIR:
            return "OTIR";

        case Instruction::OUTD:
            return "OUTD";

        case Instruction::OTDR:
            return "OTDR";
    }

    Util::debug << "unhandled instruction enumerator\n";
    abort();
}

std::string std::to_string(const Mnemonic & mnemonic)
{
    std::ostringstream out;
    out << to_string(mnemonic.instruction);
    bool first = true;

    for (const auto & operand : mnemonic.operands) {
        if (first) {
            out << ' ';
            first = false;
        } else {
            out << ',';
        }

        out << to_string(operand);
    }

    return out.str();
}
