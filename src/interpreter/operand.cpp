//
// Created by darren on 11/03/2021.
//

#include <QRegularExpression>
#include <QStringBuilder>
#include "operand.h"

using namespace Interpreter;

void Operand::parse()
{
    static QString DecimalLiteralPattern = QStringLiteral("([+-]?(?:[0-9]|[1-9][0-9]+))D?");
    static QString HexLiteralPattern = QStringLiteral("\\$([0-9A-F]+)|0X([0-9A-F]+)|([0-9A-F]+)+H");
    static QString OctalLiteralPattern = QStringLiteral("([0-7]+)O|0([0-7]+)");
    static QString BinaryLiteralPattern = QStringLiteral("%([01]+)|([01]+)B|0B([01]+)");

    static QRegularExpression DecimalLiteralMatcher("^" % DecimalLiteralPattern % "$");
    static QRegularExpression HexLiteralMatcher("^(?:" % HexLiteralPattern % ")$");
    static QRegularExpression OctalLiteralMatcher("^(?:" % OctalLiteralPattern % ")$");
    static QRegularExpression BinaryLiteralMatcher("^(?:" % BinaryLiteralPattern % ")$");

    // captures: 1 = decimal address; 2, 3, 4 = hex address; 5, 6 = octal address; 7, 8, 9 = binary address
    // only one capture will be populated, the others will have 0 length
    static QRegularExpression IndirectAddressMatcher("^\\((?:" % DecimalLiteralPattern % "|" % HexLiteralPattern % "|" % OctalLiteralPattern % "|" % BinaryLiteralPattern % ")\\)$");

    static QRegularExpression IndirectReg8Matcher("^\\(\\s*(B|C|D|E|H|L|A|F|IXH|IXL|IYH|IYL|I|R|B'|C'|D'|E'|H'|L'|A'|F')\\s*\\)$");
    static QRegularExpression IndirectReg16Matcher("^\\(\\s*(BC|DE|HL|AF|SP|PC|IX|IY|BC'|DE'|HL'|AF')\\s*\\)$");
    static QRegularExpression IndirectReg16OffsetMatcher("^\\(\\s*(IX|IY)\\s*([+\\-])\\s*([0-9]+|0[0-9]+|[01]+B|0X[0-9A-F]+|[0-9A-F]+H)\\s*\\)$");

    m_type = OperandType::InvalidOperand;
    m_number = 0;

    /* number literals */
    if (auto match = DecimalLiteralMatcher.match(m_string); match.hasMatch()) {
        /* decimal number */
        m_type = OperandType::NumberLiteral;
        m_number = match.captured(1).toInt(nullptr, 10);
    } else if (match = OctalLiteralMatcher.match(m_string); match.hasMatch()) {
        /* octal number */
        m_type = OperandType::NumberLiteral;

        if (0 < match.capturedLength(1)) {
            m_number = match.capturedRef(1).toInt(nullptr, 8);
        } else {
            m_number = match.capturedRef(2).toInt(nullptr, 8);
        }
    } else if (match = BinaryLiteralMatcher.match(m_string); match.hasMatch()) {
        /* binary number */
        m_type = OperandType::NumberLiteral;

        if (0 < match.capturedLength(1)) {
            m_number = match.capturedRef(1).toInt(nullptr, 2);
        } else if (0 < match.capturedLength(2)) {
            m_number = match.capturedRef(2).toInt(nullptr, 2);
        } else {
            m_number = match.capturedRef(3).toInt(nullptr, 2);
        }
    } else if (match = HexLiteralMatcher.match(m_string); match.hasMatch()) {
        /* hex number */
        m_type = OperandType::NumberLiteral;

        if (0 < match.capturedLength(1)) {
            m_number = match.capturedRef(1).toInt(nullptr, 16);
        } else if (0 < match.capturedLength(2)) {
            m_number = match.capturedRef(2).toInt(nullptr, 16);
        } else {
            m_number = match.capturedRef(3).toInt(nullptr, 16);
        }
    }
    else if (match = IndirectAddressMatcher.match(m_string); match.hasMatch()) {
        // indirect address literal
        m_type = OperandType::IndirectAddress;

        if (0 < match.capturedLength(1)) {
            // decimal notation
            m_number = match.capturedRef(1).toInt(nullptr, 10);
        } else if (0 < match.capturedLength(2)) {
            // hex notation
            m_number = match.capturedRef(2).toInt(nullptr, 16);
        } else if (0 < match.capturedLength(3)) {
            // hex notation
            m_number = match.capturedRef(3).toInt(nullptr, 16);
        } else if (0 < match.capturedLength(4)) {
            // decimal notation
            m_number = match.capturedRef(4).toInt(nullptr, 16);
        } else if (0 < match.capturedLength(5)) {
            // octal notation
            m_number = match.capturedRef(5).toInt(nullptr, 8);
        } else if (0 < match.capturedLength(6)) {
            // octal notation
            m_number = match.capturedRef(7).toInt(nullptr, 8);
        } else if (0 < match.capturedLength(7)) {
            // binary notation
            m_number = match.capturedRef(8).toInt(nullptr, 2);
        } else if (0 < match.capturedLength(8)) {
            // binary notation
            m_number = match.capturedRef(8).toInt(nullptr, 2);
        } else if (0 < match.capturedLength(9)) {
            // binary notation
            m_number = match.capturedRef(9).toInt(nullptr, 2);
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
    } else if (match = IndirectReg8Matcher.match(m_string); match.hasMatch()) {
        m_type = OperandType::IndirectReg8;
        auto reg = match.capturedRef(1);

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
    } else if (match = IndirectReg16Matcher.match(m_string); match.hasMatch()) {
        m_type = OperandType::IndirectReg16;
        auto reg = match.capturedRef(1);

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
    } else if (match = IndirectReg16OffsetMatcher.match(m_string); match.hasMatch()) {
        m_type = OperandType::IndirectReg16WithOffset;
        auto reg = match.capturedRef(1);

        if ("IX" == reg) {
            m_reg16 = Register16::IX;
        } else if ("IY" == reg) {
            m_reg16 = Register16::IY;
        } else {
            m_type = OperandType::InvalidOperand;
        }

        bool neg = ("-" == match.capturedRef(2));
        auto offsetString = match.capturedRef(3);
        int d;

        if (offsetString.startsWith("0X")) {
            d = offsetString.mid(2).toInt(nullptr, 16);
        } else if (offsetString.endsWith("H")) {
            d = offsetString.left(offsetString.length() - 1).toInt(nullptr, 16);
        } else if (offsetString.endsWith("B")) {
            d = offsetString.left(offsetString.length() - 1).toInt(nullptr, 2);
        } else if (offsetString.startsWith("0") && offsetString.length() > 1) {
            d = offsetString.toInt(nullptr, 8);
        } else {
            d = offsetString.toInt(nullptr, 10);
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
