//
// Created by darren on 17/03/2021.
//

#ifndef Z80_ASSEMBLY_OPERAND_H
#define Z80_ASSEMBLY_OPERAND_H

#include <cstdint>
#include "../types.h"

namespace Z80
{
    class Z80;
}

namespace Z80::Assembly
{
    enum class AddressingMode : std::uint8_t
    {
        Immediate,              // an 8-bit unsigned value
        ImmediateExtended,      // a 16-bit unsigned value
        ModifiedPageZero,       // for RST instructions - unsignedWord specifies one of the 8 valid addresses
        Relative,               // signed 8-bit address offset - signedByte contains the offset
        Extended,               // unsigned 16-bit absolute address - unsignedWord contains the address
        Indexed,                // IX or IY with 8-bit offset - indexedAddress contains the register and offset
        Register8,              // one of the 8-bit registers - see register8
        Register16,             // one of the 16-bit registers - see register16
        Register8Indirect,      // unsigned MSB of 16-bit port contained in an 8-bit register - see register8
        Register16Indirect,     // unsigned 16-bit absolute address/port contained in a 16-bit register - see register16
        Bit,                    // the bit to work with for BIT/SET/RES instructions - see bit
    };

    struct IndexedAddress
    {
        Register16 register16;
        SignedByte offset;
    };

    union OperandValue
    {
        ::Z80::UnsignedByte unsignedByte;
        ::Z80::SignedByte signedByte;
        ::Z80::UnsignedWord unsignedWord;
        ::Z80::UnsignedByte bit;
    };

    struct Operand
    {
        AddressingMode mode;

        union {
            ::Z80::Register16 register16;
            ::Z80::Register8 register8;
            ::Z80::UnsignedByte unsignedByte;
            ::Z80::SignedByte signedByte;
            ::Z80::UnsignedWord unsignedWord;
            IndexedAddress indexedAddress;
            ::Z80::UnsignedByte bit;
        };

        /**
         * Evaluate an operand based on the state of a provided Z80 CPU.
         *
         * If evaluating as a destination, the operand will be evaluated at most to an address; if evaluating as a
         * source, the operand will be evaluated to an actual value.
         *
         * For example, when evaluating (HL) as a destination, the operand will be evaluated as the address stored in
         * HL. When evaluating (HL) as a source, the operand will be evaluated to the value stored at the address in
         * HL.
         *
         * Evaluation of values at addresses will always be done using 16 bits, unless the address is 0xffff in
         * which case only 8 bits will be read and the MSB will be 0x00 (since only one byte - the LSB - is available
         * at 0xffff in the 16-bit address space of the Z80). If the destination is only 8 bits wide, the LSB of the
         * evaluated value is the actual value placed in the destination.
         *
         * In some cases it doesn't matter whether it's a source or destination (e.g. immediate addressing modes) -
         * these are usually cases where the addressing mode can only be used as a source.
         *
         * @param cpu
         * @param asDestination
         * @return
         */
        ::Z80::Assembly::OperandValue evaluate(::Z80::Z80 * cpu, bool asDestination = true) const;
    };
}

namespace std
{
    std::string to_string(const ::Z80::Assembly::Operand &);
    std::string to_string(const ::Z80::Register16 &);
    std::string to_string(const ::Z80::Register8 &);
}

#endif //Z80_ASSEMBLY_OPERAND_H
