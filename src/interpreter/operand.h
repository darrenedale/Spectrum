//
// Created by darren on 11/03/2021.
//

#ifndef INTERPRETER_OPERAND_H
#define INTERPRETER_OPERAND_H

#include <QString>

#include "../z80/types.h"

namespace Interpreter
{
    /**
     * Discriminated union representing a single operand for a Z80 instruction.
     */
    class Operand
    {
        using UnsignedByte = Z80::UnsignedByte;
        using UnsignedWord = Z80::UnsignedWord;
        using SignedByte = Z80::SignedByte;
        using SignedWord = Z80::SignedWord;
        using Register8 = Z80::Register8;
        using Register16 = Z80::Register16;

    public:
        enum class OperandType
        {
            InvalidOperand = 0,
            NumberLiteral,
            IndirectAddress,
            Register16,
            Register8,
            IndirectReg16,
            IndirectReg8,
            IndirectReg16WithOffset,
            Condition,
            Port
        };

        enum class ConditionType
        {
            InvalidCondition = 0,
            Z,
            Zero = Z,
            NZ,
            NonZero = NZ,
            C,
            Carry = C,
            NC,
            NoCarry = NC,
            PO,
            ParityOdd = PO,
            PE,
            ParityEven = PE,
            P,
            Plus = P,
            M,
            Minus = M
        };

        inline Operand(const QString & op)
                : m_string(op.trimmed().toUpper()),
                  m_type(OperandType::InvalidOperand),
                  m_number(0)
        {
            parse();
        }

        [[nodiscard]] inline OperandType type() const
        {
            return m_type;
        }

        [[nodiscard]] inline bool isValid() const
        {
            return m_type != OperandType::InvalidOperand && (m_type != OperandType::Condition || m_condition != ConditionType::InvalidCondition);
        }

        [[nodiscard]] inline bool isNumber() const
        {
            return isByte() || isWord();
        }

        [[nodiscard]] inline bool isRegister() const
        {
            return isReg16() || isReg8();
        }

        [[nodiscard]] inline bool isReg16() const
        {
            return m_type == OperandType::Register16;
        }

        [[nodiscard]] inline bool isReg8() const
        {
            return m_type == OperandType::Register8;
        }

        [[nodiscard]] inline bool isBitIndex() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= 0 && m_number <= 7;
        }

        [[nodiscard]] inline bool isByte() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= 0 && m_number <= 256;
        }

        [[nodiscard]] inline bool isSignedByte() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= -128 && m_number <= 127;
        }

        [[nodiscard]] inline bool isWord() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= 0 && m_number <= 65535;
        }

        [[nodiscard]] inline bool isSignedWord() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= -32767 && m_number <= 32768;
        }

        [[nodiscard]] inline bool isOffset() const
        {
            return isSignedByte();
        }

        [[nodiscard]] inline bool isIndirectAddress() const
        {
            return m_type == OperandType::IndirectAddress;
        }

        [[nodiscard]] inline bool isIndirectReg16() const
        {
            return m_type == OperandType::IndirectReg16;
        }

        [[nodiscard]] inline bool isIndirectReg16WithOffset() const
        {
            return m_type == OperandType::IndirectReg16WithOffset;
        }

        [[nodiscard]] inline bool isIndirectReg8() const
        {
            return m_type == OperandType::IndirectReg8;
        }

        [[nodiscard]] inline bool isCondition() const
        {
            /* "C" can be either a register or the condition Carry, so while it is stored internally as RegC, report it externally as the C condition also */
            return m_type == OperandType::Condition || (m_type == OperandType::Register8 && m_reg8 == Register8::C);
        }

        [[nodiscard]] inline bool isPort() const
        {
            return m_type == OperandType::Port;
        }

        [[nodiscard]] inline ConditionType condition() const
        {
            return m_condition;
        }

        [[nodiscard]] inline const QString & string() const
        {
            return m_string;
        }

        [[nodiscard]] inline const QString & operand() const
        {
            return m_string;
        }

        [[nodiscard]] inline UnsignedByte bitIndex() const
        {
            return static_cast<UnsignedByte>(m_number & 0x07);
        }

        [[nodiscard]] inline UnsignedByte byte() const
        {
            return static_cast<UnsignedByte>(m_number & 0xff);
        }

        [[nodiscard]] inline UnsignedWord word() const
        {
            return static_cast<UnsignedWord>(m_number & 0xffff);
        }

        [[nodiscard]] inline SignedByte signedByte() const
        {
            return static_cast<SignedByte>(m_number & 0xff);
        }

        [[nodiscard]] inline SignedWord signedWord() const
        {
            return static_cast<SignedWord>(m_number & 0xffff);
        }

        [[nodiscard]] inline SignedByte offset() const
        {
            return signedByte();
        }

        [[nodiscard]] inline UnsignedByte wordLowByte() const
        {
            return word() & 0x00ff;
        }

        [[nodiscard]]  inline UnsignedByte wordHighByte() const
        {
            return ((word() & 0xff00) >> 8);
        }

        [[nodiscard]]  inline Z80::Register16 reg16() const
        {
            return m_reg16;
        }

        [[nodiscard]]  inline Z80::Register8 reg8() const
        {
            return m_reg8;
        }

        [[nodiscard]]  inline UnsignedWord address() const
        {
            return static_cast<UnsignedWord>(m_number & 0xffff);
        }

    private:
        void parse();

        QString m_string;
        OperandType m_type;

        union
        {
            int m_number;
            Z80::Register16 m_reg16;
            Z80::Register8 m_reg8;
            ConditionType m_condition;
        };
    };
}

#endif //INTERPRETER_OPERAND_H
