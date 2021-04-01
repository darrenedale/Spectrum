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
    using UnsignedByte = std::uint8_t;
    using UnsignedWord = std::uint16_t;
    using SignedByte = std::int8_t;
    using SignedWord = std::int16_t;

    enum class Register16 : std::uint8_t
    {
        AF, BC, DE, HL,
        IX, IY,
        SP, PC,
        AFShadow, BCShadow, DEShadow, HLShadow
    };

    enum class Register8 : std::uint8_t
    {
        A, F, B, C, D, E, H, L,
        IXH, IXL, IYH, IYL,
        I, R,
        AShadow, FShadow, BShadow, CShadow, DShadow, EShadow, HShadow, LShadow
    };

    enum class InterruptMode : std::uint8_t
    {
        IM0 = 0,
        IM1,
        IM2,
    };

    constexpr const std::endian HostByteOrder = std::endian::native;
    constexpr const std::endian Z80ByteOrder = std::endian::little;
}


namespace std
{
    std::string to_string(const ::Z80::Register16 &);
    std::string to_string(const ::Z80::Register8 &);
}

#endif //Z80_TYPES_H
