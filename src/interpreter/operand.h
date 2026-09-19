//
// Created by darren on 11/03/2021.
//

#ifndef INTERPRETER_OPERAND_H
#define INTERPRETER_OPERAND_H

#include <string>

#include "../z80/types.h"
#include "../util/string.h"

namespace Interpreter
{
    /** Discriminated union representing a single operand for a Z80 instruction. */
    class Operand
    {
        using UnsignedByte = Z80::UnsignedByte;
        using UnsignedWord = Z80::UnsignedWord;
        using SignedByte = Z80::SignedByte;
        using SignedWord = Z80::SignedWord;
        using Register8 = Z80::Register8;
        using Register16 = Z80::Register16;

    public:
        /** Enumeration of the possible operand types. */
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

        /** Enumeration of the possible condition types. */
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

        /**
         * Initialise an operand by parsing its string representation.
         *
         * @param op The operand string.
         */
        explicit Operand(const std::string & op) noexcept
        : m_string(std::move(Util::upper_cased(Util::trimmed(op)))),
          m_type(OperandType::InvalidOperand),
          m_number(0)
        {
            parse();
        }

        /**
         * Fetch the operand type.
         *
         * @return The operand type.
         */
        [[nodiscard]]
        OperandType type() const noexcept
        {
            return m_type;
        }

        /**
         * Determine whether the operand is valid.
         *
         * A valid operand has a known type and if it's a condition it also has a known condition type.
         *
         * @return true if it's valid, false otherwise.
         */
        [[nodiscard]]
        bool isValid() const noexcept
        {
            return
                OperandType::InvalidOperand != m_type
                && (
                    OperandType::Condition != m_type
                    || ConditionType::InvalidCondition != m_condition
                );
        }

        /**
         * Check whether the operand is a numeric literal.
         *
         * @return true if it's a byte or a word, false otherwise.
         */
        [[nodiscard]]
        bool isNumber() const noexcept
        {
            return isByte() || isWord();
        }

        /**
         * Check whether the operand is a register.
         *
         * @return true if it's one of the 8-bit or 16-bit registers, false otherwise.
         */
        [[nodiscard]]
        bool isRegister() const noexcept
        {
            return isReg16() || isReg8();
        }

        /**
         * Check whether the operand is a 16-bit register pair.
         *
         * @return true if it's one of the 16-bit registers, false otherwise.
         */
        [[nodiscard]]
        bool isReg16() const noexcept
        {
            return OperandType::Register16 == m_type;
        }

        /**
         * Check whether the operand is an 8-bit register.
         *
         * @return true if it's one of the 8-bit registers, false otherwise.
         */
        [[nodiscard]]
        bool isReg8() const noexcept
        {
            return OperandType::Register8 == m_type;
        }

        /**
         * Check whether the operand is a bit index.
         *
         * @return true if it's a bit index in one of the Z80 bit-handling instructions, false otherwise.
         */
        [[nodiscard]]
        bool isBitIndex() const noexcept
        {
            return OperandType::NumberLiteral == m_type && 0 <= m_number && 7 >= m_number;
        }

        /**
         * Check whether the operand is a literal byte.
         *
         * @return true if it's a byte literal, false otherwise.
         */
        [[nodiscard]]
        bool isByte() const noexcept
        {
            return OperandType::NumberLiteral == m_type && 0 <= m_number && 256 >= m_number;
        }

        /**
         * Check whether the operand is a signed literal byte.
         *
         * @return true if it's a signed byte literal, false otherwise.
         */
        [[nodiscard]]
        bool isSignedByte() const noexcept
        {
            return OperandType::NumberLiteral == m_type && -128 <= m_number && 127 >= m_number;
        }

        /**
         * Check whether the operand is a literal 16-bit word.
         *
         * @return true if it's a 16-bit word literal, false otherwise.
         */
        [[nodiscard]]
        bool isWord() const noexcept
        {
            return OperandType::NumberLiteral == m_type && 0 <= m_number && 65535 >= m_number;
        }

        /**
         * Check whether the operand is a literal 16-bit signed word.
         *
         * @return true if it's a 16-bit signed word literal, false otherwise.
         */
        [[nodiscard]]
        bool isSignedWord() const noexcept
        {
            return OperandType::NumberLiteral == m_type && -32767 <= m_number && 32768 >= m_number;
        }

        /**
         * Check whether the operand is an offset.
         *
         * @return true if it's an offset for use with a Z80 index register instruction, false otherwise.
         */
        [[nodiscard]]
        bool isOffset() const noexcept
        {
            return isSignedByte();
        }

        /**
         * Check whether the operand is an indirect literal address.
         *
         * @return true if it's an indirect address literal, false otherwise.
         */
        [[nodiscard]]
        bool isIndirectAddress() const noexcept
        {
            return OperandType::IndirectAddress == m_type;
        }

        /**
         * Check whether the operand is an indirect 16-bit register pair.
         *
         * @return true if it's an indirect 16-bit register pair, false otherwise.
         */
        [[nodiscard]]
        bool isIndirectReg16() const noexcept
        {
            return OperandType::IndirectReg16 == m_type;
        }

        /**
         * Check whether the operand is an indirect 16-bit register pair with an offset.
         *
         * @return true if it's an indirect 16-bit index register pair with an offset, false otherwise.
         */
        [[nodiscard]]
        bool isIndirectReg16WithOffset() const noexcept
        {
            return OperandType::IndirectReg16WithOffset == m_type;
        }

        /**
         * Check whether the operand is an indirect 8-bit register.
         *
         * @return true if it's an indirect 8-bit register, false otherwise.
         */
        [[nodiscard]]
        bool isIndirectReg8() const noexcept
        {
            return OperandType::IndirectReg8 == m_type;
        }

        /**
         * Check whether the operand is a condition code.
         *
         * @return true if it's a Z80 condition code, false otherwise.
         */
        [[nodiscard]]
        bool isCondition() const noexcept
        {
            /*
             * "C" can be either a register or the condition Carry, so while it is stored internally as RegC, report it
             * externally as the C condition also
             */
            return OperandType::Condition == m_type || (OperandType::Register8 == m_type && Register8::C == m_reg8);
        }

        /**
         * Check whether the operand is a port number.
         *
         * @return true if it's a port number, false otherwise.
         */
        [[nodiscard]]
        bool isPort() const noexcept
        {
            return OperandType::Port == m_type;
        }

        /**
         * Fetch the condition if the operand is a condition type.
         *
         * @return The condition type. This is undefined if the operand is not a condition type.
         */
        [[nodiscard]]
        ConditionType condition() const noexcept
        {
            return m_condition;
        }

        /**
         * Fetch the string representation of the operand.
         *
         * @return The string representation of the operand.
         */
        [[nodiscard]]
        const std::string & string() const noexcept
        {
            return m_string;
        }

        /**
         * Fetch the string representation of the operand.
         *
         * Alias of string().
         *
         * @return The string representation of the operand.
         */
        [[nodiscard]]
        const std::string & operand() const noexcept
        {
            return m_string;
        }

        /**
         * Fetch the bit index.
         *
         * @return The index if the operand is a bit index, undefined otherwise.
         */
        [[nodiscard]]
        UnsignedByte bitIndex() const noexcept
        {
            return static_cast<UnsignedByte>(m_number & 0x07);
        }

        /**
         * Fetch the byte value.
         *
         * @return The value if the operand is a byte literal, undefined otherwise.
         */
        [[nodiscard]]
        UnsignedByte byte() const noexcept
        {
            return static_cast<UnsignedByte>(m_number & 0xff);
        }

        /**
         * Fetch the word value.
         *
         * @return The value if the operand is a 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]]
        UnsignedWord word() const noexcept
        {
            return static_cast<UnsignedWord>(m_number & 0xffff);
        }

        /**
         * Fetch the signed byte value.
         *
         * @return The value if the operand is a signed byte literal, undefined otherwise.
         */
        [[nodiscard]]
        SignedByte signedByte() const noexcept
        {
            return static_cast<SignedByte>(m_number & 0xff);
        }

        /**
         * Fetch the signed word value.
         *
         * @return The value if the operand is a signed 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]]
        SignedWord signedWord() const noexcept
        {
            return static_cast<SignedWord>(m_number & 0xffff);
        }

        /**
         * Fetch the offset.
         *
         * @return The offset if the operand is an offset for use with an index register instruction, undefined otherwise.
         */
        [[nodiscard]]
        SignedByte offset() const noexcept
        {
            return signedByte();
        }

        /**
         * Fetch the low byte of the 16-bit word value.
         *
         * @return The low byte of the value if the operand is a 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]]
        UnsignedByte wordLowByte() const noexcept
        {
            return word() & 0x00ff;
        }

        /**
         * Fetch the high byte of the 16-bit word value.
         *
         * @return The high byte of the value if the operand is a 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]]
        UnsignedByte wordHighByte() const noexcept
        {
            return (word() & 0xff00) >> 8;
        }

        /**
         * Fetch the 16-bit register pair.
         *
         * @return The register pair if the operand is a 16-bit register pair, undefined otherwise.
         */
        [[nodiscard]]
        Z80::Register16 reg16() const noexcept
        {
            return m_reg16;
        }

        /**
         * Fetch the 8-bit register.
         *
         * @return The register if the operand is an 8-bit register, undefined otherwise.
         */
        [[nodiscard]]
        Z80::Register8 reg8() const noexcept
        {
            return m_reg8;
        }

        /**
         * Fetch the address.
         *
         * @return The address if the operand is an address, undefined otherwise.
         */
        [[nodiscard]]
        UnsignedWord address() const noexcept
        {
            return static_cast<UnsignedWord>(m_number & 0xffff);
        }

    private:
        /** Helper to parse the string representation of the operand. */
        void parse() noexcept;

        /** The original string representation of the operand. */
        std::string m_string;

        /**
         * The parse operand type.
         *
         * This is the discriminator for the union.
         */
        OperandType m_type;

        union
        {
            /**
             * The number if the operand is a numeric literal of some kind. This includes ports, addresses, bit indexes,
             * etc.
             */
            int m_number;

            /** The register pair, if the operand is a 16-bit register pair. */
            Z80::Register16 m_reg16;

            /** The register, if the operand is an 8-bit register. */
            Z80::Register8 m_reg8;

            /** The condition type if the operand is a Z80 condition code. */
            ConditionType m_condition;
        };
    };
}

#endif //INTERPRETER_OPERAND_H
