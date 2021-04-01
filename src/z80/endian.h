//
// Created by darren on 01/04/2021.
//

#ifndef Z80_ENDIAN_H
#define Z80_ENDIAN_H

#include "types.h"

namespace Z80
{
    static inline constexpr UnsignedWord swapByteOrder(UnsignedWord value)
    {
        return (((value & 0xff00) >> 8) & 0x00ff) | (((value & 0x00ff) << 8) & 0xff00);
    }

    static inline constexpr UnsignedWord z80ToHostByteOrder(UnsignedWord value)
    {
        if constexpr (Z80ByteOrder == HostByteOrder) {
            return value;
        }

        return swapByteOrder(value);
    }

    static inline constexpr UnsignedWord hostToZ80ByteOrder(UnsignedWord value)
    {
        if constexpr (Z80ByteOrder == HostByteOrder) {
            return value;
        }

        return swapByteOrder(value);
    }

}

#endif //Z80_ENDIAN_H
