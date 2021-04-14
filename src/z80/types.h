//
// Created by darren on 27/02/2021.
//

#ifndef Z80_TYPES_H
#define Z80_TYPES_H

#include <cstdint>
#include <bit>
#include <string>

namespace Z80
{
    /**
     * A Z80 8-bit unsigned value.
     */
    using UnsignedByte = std::uint8_t;

    /**
     * A Z80 16-bit unsigned value.
     *
     * Note that values of this type do not guarantee Z80 byte order.
     */
    using UnsignedWord = std::uint16_t;

    /**
     * A Z80 8-bit signed value.
     */
    using SignedByte = std::int8_t;

    /**
     * A Z80 16-bit signed value.
     *
     * Note that values of this type do not guarantee Z80 byte order.
     */
    using SignedWord = std::int16_t;

    /**
     * Enumeration of the available Z80 16-bit register pairs.
     */
    enum class Register16 : std::uint8_t
    {
        AF, BC, DE, HL,
        IX, IY,
        SP, PC,
        AFShadow, BCShadow, DEShadow, HLShadow
    };

    /**
     * Enumeration of the available Z80 8-bit registers.
     *
     * Note that some of these can't be addressed directly, but they physically exist.
     */
    enum class Register8 : std::uint8_t
    {
        A, F, B, C, D, E, H, L,
        IXH, IXL, IYH, IYL,
        I, R,
        AShadow, FShadow, BShadow, CShadow, DShadow, EShadow, HShadow, LShadow
    };

    /**
     * Enumeration of the possible Z80 interrupt modes.
     */
    enum class InterruptMode : std::uint8_t
    {
        IM0 = 0,
        IM1,
        IM2,
    };

    struct InstructionCost
    {
        std::uint8_t tStates;   // number of t-states the instruction took to executed
        std::uint8_t size;      // size in bytes of the instruction
    };

    constexpr const std::endian HostByteOrder = std::endian::native;
    constexpr const std::endian Z80ByteOrder = std::endian::little;
}

namespace std // NOLINT(cert-dcl58-cpp) only to_string() is overloaded and only with our namespaced types
{
    /**
     * Provide a string representation of a 16-bit register pair name.
     *
     * @return
     */
    std::string to_string(const ::Z80::Register16 &);

    /**
     * Provide a string representation of an 8-bit register name.
     *
     * @return
     */
    std::string to_string(const ::Z80::Register8 &);

    /**
     * Provide a string representation of a interrupt mode.
     * @return
     */
    std::string to_string(const ::Z80::InterruptMode &);
}

/**
 * User-defined literal for Z80::UnsignedByte
 *
 * E.g., 0xff_z80ub is a Z80::UnsignedByte literal (value 255)
 *
 * @param value
 * @return
 */
constexpr Z80::UnsignedByte operator "" _z80ub(unsigned long long value) noexcept
{
    return static_cast<Z80::UnsignedByte>(value);
}

/**
 * User-defined literal for Z80::SignedByte
 *
 * E.g., 0xff_z80sb is a Z80::UnsignedByte literal (value -127)
 *
 * @param value
 * @return
 */
constexpr Z80::SignedByte operator "" _z80sb(unsigned long long value) noexcept
{
    return static_cast<Z80::SignedByte>(value);
}

/**
 * User-defined literal for Z80::UnsignedWord
 *
 * E.g., 0xffff_z80sb is a Z80::UnsignedWord literal (value 65535)
 *
 * Note that no byte-order conversion is performed, this is simply a way of ensuring a numeric literal value is cast to
 * the underlying type being used for 16-bit Z80 values without having to either know the underlying type or write an
 * explicit cast. The value will remain in host byte order.
 *
 * @param value
 * @return
 */
constexpr Z80::UnsignedWord operator "" _z80uw(unsigned long long value) noexcept
{
    return static_cast<Z80::UnsignedWord>(value);
}

/**
 * User-defined literal for Z80::SignedWord
 *
 * E.g., 0xffff_z80sb is a Z80::SignedWord literal (value -32767)
 *
 * Note that no byte-order conversion is performed, this is simply a way of ensuring a numeric literal value is cast to
 * the underlying type being used for 16-bit Z80 values without having to either know the underlying type or write an
 * explicit cast. The value will remain in host byte order.
 *
 * @param value
 * @return
 */
constexpr Z80::SignedWord operator "" _z80sw(unsigned long long value) noexcept
{
    return static_cast<Z80::SignedWord>(value);
}

#endif //Z80_TYPES_H
