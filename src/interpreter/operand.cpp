//
// Created by darren on 11/03/2021.
//

#include <regex>
#include "operand.h"

using namespace Interpreter;

void Operand::parse()
{
    static constexpr const char * DecimalLiteralPattern = "([+-]?(?:[0-9]|[1-9][0-9]+))D?";
    static constexpr const char * HexLiteralPattern = "\\$([0-9A-F]+)|0X([0-9A-F]+)|([0-9A-F]+)+H";
    static constexpr const char * OctalLiteralPattern = "([0-7]+)O|0([0-7]+)";
    static constexpr const char * BinaryLiteralPattern = "%([01]+)|([01]+)B|0B([01]+)";

    static std::regex DecimalLiteralMatcher(std::string("^") + DecimalLiteralPattern + "$");
    static std::regex HexLiteralMatcher(std::string("^(?:") + HexLiteralPattern + ")$");
    static std::regex OctalLiteralMatcher(std::string("^(?:") + OctalLiteralPattern + ")$");
    static std::regex BinaryLiteralMatcher(std::string("^(?:") + BinaryLiteralPattern + ")$");

    // captures: 1 = decimal address; 2, 3, 4 = hex address; 5, 6 = octal address; 7, 8, 9 = binary address
    // only one capture will be populated, the others will have 0 length
    static std::regex IndirectAddressMatcher(std::string("^\\((?:") + DecimalLiteralPattern + "|" + HexLiteralPattern + "|" + OctalLiteralPattern + "|" + BinaryLiteralPattern + ")\\)$");

    static std::regex IndirectReg8Matcher("^\\(\\s*(B|C|D|E|H|L|A|F|IXH|IXL|IYH|IYL|I|R|B'|C'|D'|E'|H'|L'|A'|F')\\s*\\)$");
    static std::regex IndirectReg16Matcher("^\\(\\s*(BC|DE|HL|AF|SP|PC|IX|IY|BC'|DE'|HL'|AF')\\s*\\)$");
    static std::regex IndirectReg16OffsetMatcher("^\\(\\s*(IX|IY)\\s*([+\\-])\\s*([0-9]+|0[0-9]+|[01]+B|0X[0-9A-F]+|[0-9A-F]+H)\\s*\\)$");

    m_type = OperandType::InvalidOperand;
    m_number = 0;

    /* number literals */
    std::smatch match;

    if (std::regex_match(m_string, match, DecimalLiteralMatcher)) {
        /* decimal number */
        m_type = OperandType::NumberLiteral;
        m_number = std::stoi(match[1]);
    } else if (std::regex_match(m_string, match, OctalLiteralMatcher)) {
        /* octal number */
        m_type = OperandType::NumberLiteral;

        if (0 < match[1].length()) {
            m_number = std::stoi(match[1], nullptr, 8);
        } else {
            m_number = std::stoi(match[2], nullptr, 8);
        }
    } else if (std::regex_match(m_string, match, BinaryLiteralMatcher)) {
        /* binary number */
        m_type = OperandType::NumberLiteral;

        if (0 < match[1].length()) {
            m_number = std::stoi(match[1], nullptr, 2);
        } else if (0 < match[2].length()) {
            m_number = std::stoi(match[2], nullptr, 2);
        } else {
            m_number = std::stoi(match[3], nullptr, 2);
        }
    } else if (std::regex_match(m_string, match, HexLiteralMatcher)) {
        /* hex number */
        m_type = OperandType::NumberLiteral;

        if (0 < match[1].length()) {
            m_number = std::stoi(match[1], nullptr, 16);
        } else if (0 < match[2].length()) {
            m_number = std::stoi(match[2], nullptr, 16);
        } else {
            m_number = std::stoi(match[3], nullptr, 16);
        }
    }
    else if (std::regex_match(m_string, match, IndirectAddressMatcher)) {
        // indirect address literal
        m_type = OperandType::IndirectAddress;

        if (0 < match[1].length()) {
            // decimal notation
            m_number = std::stoi(match[1], nullptr, 10);
        } else if (0 < match[2].length()) {
            // hex notation
            m_number = std::stoi(match[2], nullptr, 16);
        } else if (0 < match[3].length()) {
            // hex notation
            m_number = std::stoi(match[3], nullptr, 16);
        } else if (0 < match[4].length()) {
            // decimal notation
            m_number = std::stoi(match[4], nullptr, 16);
        } else if (0 < match[5].length()) {
            // octal notation
            m_number = std::stoi(match[5], nullptr, 8);
        } else if (0 < match[6].length()) {
            // octal notation
            m_number = std::stoi(match[7], nullptr, 8);
        } else if (0 < match[7].length()) {
            // binary notation
            m_number = std::stoi(match[8], nullptr, 2);
        } else if (0 < match[8].length()) {
            // binary notation
            m_number = std::stoi(match[8], nullptr, 2);
        } else if (0 < match[9].length()) {
            // binary notation
            m_number = std::stoi(match[9], nullptr, 2);
        }
    } else if ("BC" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::BC;
    } else if ("DE" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::DE;
    } else if ("HL" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::HL;
    } else if ("AF" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::AF;
    } else if ("IX" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::IX;
    } else if ("IY" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::IY;
    } else if ("SP" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::SP;
    } else if ("PC" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::PC;
    } else if ("BC'" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::BCShadow;
    } else if ("DE'" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::DEShadow;
    } else if ("HL'" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::HLShadow;
    } else if ("AF'" == m_string) {
        m_type = OperandType::Register16;
        m_reg16 = Register16::AFShadow;
    } else if ("B" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::B;
    } else if ("C" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::C;
    } else if ("D" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::D;
    } else if ("E" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::E;
    } else if ("H" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::H;
    } else if ("L" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::L;
    } else if ("A" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::A;
    } else if ("F" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::F;
    } else if ("I" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::I;
    } else if ("R" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::R;
    } else if ("B'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::BShadow;
    } else if ("C'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::CShadow;
    } else if ("D'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::DShadow;
    } else if ("E'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::EShadow;
    } else if ("H'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::HShadow;
    } else if ("L'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::LShadow;
    } else if ("A'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::AShadow;
    } else if ("F'" == m_string) {
        m_type = OperandType::Register8;
        m_reg8 = Register8::FShadow;
    } else if ("Z" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::Zero;
    } else if ("NZ" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::NonZero;
    } else if ("NC" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::NoCarry;
    } else if ("PO" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::ParityOdd;
    } else if ("PE" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::ParityEven;
    } else if ("P" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::Plus;
    } else if ("M" == m_string) {
        m_type = OperandType::Condition;
        m_condition = ConditionType::Minus;
    } else if (std::regex_match(m_string, match, IndirectReg8Matcher)) {
        m_type = OperandType::IndirectReg8;
        const auto & reg = match[1];

        if ("B" == reg) {
            m_reg8 = Register8::B;
        } else if ("C" == reg) {
            m_reg8 = Register8::C;
        } else if ("D" == reg) {
            m_reg8 = Register8::D;
        } else if ("E" == reg) {
            m_reg8 = Register8::E;
        } else if ("H" == reg) {
            m_reg8 = Register8::H;
        } else if ("L" == reg) {
            m_reg8 = Register8::L;
        } else if ("IXH" == reg) {
            m_reg8 = Register8::IXH;
        } else if ("IXL" == reg) {
            m_reg8 = Register8::IXL;
        } else if ("IYH" == reg) {
            m_reg8 = Register8::IYH;
        } else if ("IYL" == reg) {
            m_reg8 = Register8::IYL;
        } else if ("A" == reg) {
            m_reg8 = Register8::A;
        } else if ("F" == reg) {
            m_reg8 = Register8::F;
        } else if ("I" == reg) {
            m_reg8 = Register8::I;
        } else if ("R" == reg) {
            m_reg8 = Register8::R;
        } else if ("B'" == reg) {
            m_reg8 = Register8::BShadow;
        } else if ("C'" == reg) {
            m_reg8 = Register8::CShadow;
        } else if ("D'" == reg) {
            m_reg8 = Register8::DShadow;
        } else if ("E'" == reg) {
            m_reg8 = Register8::EShadow;
        } else if ("H'" == reg) {
            m_reg8 = Register8::HShadow;
        } else if ("L'" == reg) {
            m_reg8 = Register8::LShadow;
        } else if ("A'" == reg) {
            m_reg8 = Register8::AShadow;
        } else if ("F'" == reg) {
            m_reg8 = Register8::FShadow;
        }
    } else if (std::regex_match(m_string, match, IndirectReg16Matcher)) {
        m_type = OperandType::IndirectReg16;
        const auto & reg = match[1];

        if ("BC" == reg) {
            m_reg16 = Register16::BC;
        } else if ("DE" == reg) {
            m_reg16 = Register16::DE;
        } else if ("HL" == reg) {
            m_reg16 = Register16::HL;
        } else if ("AF" == reg) {
            m_reg16 = Register16::AF;
        } else if ("IX" == reg) {
            m_reg16 = Register16::IX;
        } else if ("IY" == reg) {
            m_reg16 = Register16::IY;
        } else if ("SP" == reg) {
            m_reg16 = Register16::SP;
        } else if ("PC" == reg) {
            m_reg16 = Register16::PC;
        } else if ("BC'" == reg) {
            m_reg16 = Register16::BCShadow;
        } else if ("DE'" == reg) {
            m_reg16 = Register16::DEShadow;
        } else if ("HL'" == reg) {
            m_reg16 = Register16::HLShadow;
        } else if ("AF'" == reg) {
            m_reg16 = Register16::AFShadow;
        }
    } else if (std::regex_match(m_string, match, IndirectReg16OffsetMatcher)) {
        m_type = OperandType::IndirectReg16WithOffset;
        const auto & reg = match[1];

        if ("IX" == reg) {
            m_reg16 = Register16::IX;
        } else if ("IY" == reg) {
            m_reg16 = Register16::IY;
        } else {
            m_type = OperandType::InvalidOperand;
        }

        bool neg = ("-" == match[2]);
        const auto & offsetMatch = match[3];
        int d;

        if ("0X" == std::string_view(offsetMatch.first, offsetMatch.first + 2)) {
            d = std::stoi(std::string(offsetMatch.first + 2, offsetMatch.second), nullptr, 16);
        } else if ('H' == *(offsetMatch.second - 1)) {
            d = std::stoi(std::string(offsetMatch.first, offsetMatch.second - 1), nullptr, 16);
        } else if ('B' == *(offsetMatch.second - 1)) {
            d = std::stoi(std::string(offsetMatch.first, offsetMatch.second - 1), nullptr, 2);
        } else if ('0' == *offsetMatch.first && 1 < offsetMatch.length()) {
            d = std::stoi(offsetMatch, nullptr, 8);
        } else {
            d = std::stoi(static_cast<std::string>(offsetMatch), nullptr, 10);
        }

        if (neg) {
            d = 0 - d;
        }
        if (d < -128 || d > 127) {
            m_type = OperandType::InvalidOperand;
        } else {
            m_number = d;
        }
    }
}
