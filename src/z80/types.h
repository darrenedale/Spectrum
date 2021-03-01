//
// Created by darren on 27/02/2021.
//

#ifndef SPECTRUM_Z80_TYPES_H
#define SPECTRUM_Z80_TYPES_H

#include <cstdint>
#include <bit>

namespace Z80
{
    using UnsignedByte = std::uint8_t;
    using UnsignedWord = std::uint16_t;
    using SignedByte = std::int8_t;
    using SignedWord = std::int16_t;

    constexpr const std::endian HostByteOrder = std::endian::native;
    constexpr const std::endian Z80ByteOrder = std::endian::little;
}

#endif //SPECTRUM_Z80_TYPES_H
