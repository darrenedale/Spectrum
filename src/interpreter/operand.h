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
        /**
         * Enumeration of the possible operand types.
         */
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

        /**
         * Enumeration of the possible condition types.
         */
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
        inline explicit Operand(const std::string & op)
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
        [[nodiscard]] inline OperandType type() const
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
        [[nodiscard]] inline bool isValid() const
        {
            return m_type != OperandType::InvalidOperand && (m_type != OperandType::Condition || m_condition != ConditionType::InvalidCondition);
        }

        /**
         * Check whether the operand is a numeric literal.
         *
         * @return true if it's a byte or a word, false otherwise.
         */
        [[nodiscard]] inline bool isNumber() const
        {
            return isByte() || isWord();
        }

        /**
         * Check whether the operand is a register.
         *
         * @return true if it's one of the 8-bit or 16-bit registers, false otherwise.
         */
        [[nodiscard]] inline bool isRegister() const
        {
            return isReg16() || isReg8();
        }

        /**
         * Check whether the operand is a 16-bit register pair.
         *
         * @return true if it's one of the 16-bit registers, false otherwise.
         */
        [[nodiscard]] inline bool isReg16() const
        {
            return m_type == OperandType::Register16;
        }

        /**
         * Check whether the operand is an 8-bit register.
         *
         * @return true if it's one of the 8-bit registers, false otherwise.
         */
        [[nodiscard]] inline bool isReg8() const
        {
            return m_type == OperandType::Register8;
        }

        /**
         * Check whether the operand is a bit index.
         *
         * @return true if it's a bit index in one of the Z80 bit-handling instructions, false otherwise.
         */
        [[nodiscard]] inline bool isBitIndex() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= 0 && m_number <= 7;
        }

        /**
         * Check whether the operand is a literal byte.
         *
         * @return true if it's a byte literal, false otherwise.
         */
        [[nodiscard]] inline bool isByte() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= 0 && m_number <= 256;
        }

        /**
         * Check whether the operand is a signed literal byte.
         *
         * @return true if it's a signed byte literal, false otherwise.
         */
        [[nodiscard]] inline bool isSignedByte() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= -128 && m_number <= 127;
        }

        /**
         * Check whether the operand is a literal 16-bit word.
         *
         * @return true if it's a 16-bit word literal, false otherwise.
         */
        [[nodiscard]] inline bool isWord() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= 0 && m_number <= 65535;
        }

        /**
         * Check whether the operand is a literal 16-bit signed word.
         *
         * @return true if it's a 16-bit signed word literal, false otherwise.
         */
        [[nodiscard]] inline bool isSignedWord() const
        {
            return m_type == OperandType::NumberLiteral && m_number >= -32767 && m_number <= 32768;
        }

        /**
         * Check whether the operand is an offset.
         *
         * @return true if it's an offset for use with a Z80 index register instruction, false otherwise.
         */
        [[nodiscard]] inline bool isOffset() const
        {
            return isSignedByte();
        }

        /**
         * Check whether the operand is an indirect literal address.
         *
         * @return true if it's an indirect address literal, false otherwise.
         */
        [[nodiscard]] inline bool isIndirectAddress() const
        {
            return m_type == OperandType::IndirectAddress;
        }

        /**
         * Check whether the operand is an indirect 16-bit register pair.
         *
         * @return true if it's an indirect 16-bit register pair, false otherwise.
         */
        [[nodiscard]] inline bool isIndirectReg16() const
        {
            return m_type == OperandType::IndirectReg16;
        }

        /**
         * Check whether the operand is an indirect 16-bit register pair with an offset.
         *
         * @return true if it's an indirect 16-bit index register pair with an offset, false otherwise.
         */
        [[nodiscard]] inline bool isIndirectReg16WithOffset() const
        {
            return m_type == OperandType::IndirectReg16WithOffset;
        }

        /**
         * Check whether the operand is an indirect 8-bit register.
         *
         * @return true if it's an indirect 8-bit register, false otherwise.
         */
        [[nodiscard]] inline bool isIndirectReg8() const
        {
            return m_type == OperandType::IndirectReg8;
        }

        /**
         * Check whether the operand is a condition code.
         *
         * @return true if it's a Z80 condition code, false otherwise.
         */
        [[nodiscard]] inline bool isCondition() const
        {
            /* "C" can be either a register or the condition Carry, so while it is stored internally as RegC, report it externally as the C condition also */
            return m_type == OperandType::Condition || (m_type == OperandType::Register8 && m_reg8 == Register8::C);
        }

        /**
         * Check whether the operand is a port number.
         *
         * @return true if it's a port number, false otherwise.
         */
        [[nodiscard]] inline bool isPort() const
        {
            return m_type == OperandType::Port;
        }

        /**
         * Fetch the condition if the operand is a condition type.
         *
         * @return The condition type. This is undefined if the operand is not a condition type.
         */
        [[nodiscard]] inline ConditionType condition() const
        {
            return m_condition;
        }

        /**
         * Fetch the string representation of the operand.
         *
         * @return The string representation of the operand.
         */
        [[nodiscard]] inline const std::string & string() const
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
        [[nodiscard]] inline const std::string & operand() const
        {
            return m_string;
        }

        /**
         * Fetch the bit index.
         *
         * @return The index if the operand is a bit index, undefined otherwise.
         */
        [[nodiscard]] inline UnsignedByte bitIndex() const
        {
            return static_cast<UnsignedByte>(m_number & 0x07);
        }

        /**
         * Fetch the byte value.
         *
         * @return The value if the operand is a byte literal, undefined otherwise.
         */
        [[nodiscard]] inline UnsignedByte byte() const
        {
            return static_cast<UnsignedByte>(m_number & 0xff);
        }

        /**
         * Fetch the word value.
         *
         * @return The value if the operand is a 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]] inline UnsignedWord word() const
        {
            return static_cast<UnsignedWord>(m_number & 0xffff);
        }

        /**
         * Fetch the signed byte value.
         *
         * @return The value if the operand is a signed byte literal, undefined otherwise.
         */
        [[nodiscard]] inline SignedByte signedByte() const
        {
            return static_cast<SignedByte>(m_number & 0xff);
        }

        /**
         * Fetch the signed word value.
         *
         * @return The value if the operand is a signed 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]] inline SignedWord signedWord() const
        {
            return static_cast<SignedWord>(m_number & 0xffff);
        }

        /**
         * Fetch the offset.
         *
         * @return The offset if the operand is an offset for use with an index register instruction, undefined otherwise.
         */
        [[nodiscard]] inline SignedByte offset() const
        {
            return signedByte();
        }

        /**
         * Fetch the low byte of the 16-bit word value.
         *
         * @return The low byte of the value if the operand is a 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]] inline UnsignedByte wordLowByte() const
        {
            return word() & 0x00ff;
        }

        /**
         * Fetch the high byte of the 16-bit word value.
         *
         * @return The high byte of the value if the operand is a 16-bit word literal, undefined otherwise.
         */
        [[nodiscard]]  inline UnsignedByte wordHighByte() const
        {
            return ((word() & 0xff00) >> 8);
        }

        /**
         * Fetch the 16-bit register pair.
         *
         * @return The register pair if the operand is a 16-bit register pair, undefined otherwise.
         */
        [[nodiscard]]  inline Z80::Register16 reg16() const
        {
            return m_reg16;
        }

        /**
         * Fetch the 8-bit register.
         *
         * @return The register if the operand is an 8-bit register, undefined otherwise.
         */
        [[nodiscard]]  inline Z80::Register8 reg8() const
        {
            return m_reg8;
        }

        /**
         * Fetch the address.
         *
         * @return The address if the operand is an address, undefined otherwise.
         */
        [[nodiscard]]  inline UnsignedWord address() const
        {
            return static_cast<UnsignedWord>(m_number & 0xffff);
        }

    private:
        /**
         * Helper to parse the string representation of the operand.
         */
        void parse();

        /**
         * The original string representation of the operand.
         */
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
             * The number if the operand is a numeric literal of some kind. This includes ports, addresses, bit indexes, etc.
             */
            int m_number;

            /**
             * The register pair, if the operand is a 16-bit register pair.
             */
            Z80::Register16 m_reg16;

            /**
             * The register, if the operand is an 8-bit register.
             */
            Z80::Register8 m_reg8;

            /**
             * The condition type if the operand is a Z80 condition code.
             */
            ConditionType m_condition;
        };
    };
}

#endif //INTERPRETER_OPERAND_H
