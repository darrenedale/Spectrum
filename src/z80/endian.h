//
// Created by darren on 01/04/2021.
//

#ifndef Z80_ENDIAN_H
#define Z80_ENDIAN_H

#include "types.h"
#include "../util/endian.h"

namespace Z80
{
    using Util::swapByteOrder;

    constexpr UnsignedWord z80ToHostByteOrder(const UnsignedWord value) noexcept
    {
        if constexpr (Z80ByteOrder == HostByteOrder) {
            return value;
        }

        return swapByteOrder(value);
    }

    constexpr UnsignedWord hostToZ80ByteOrder(const UnsignedWord value) noexcept
    {
        if constexpr (Z80ByteOrder == HostByteOrder) {
            return value;
        }

        return swapByteOrder(value);
    }

}

#endif //Z80_ENDIAN_H
