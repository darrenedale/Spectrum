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

    inline constexpr UnsignedWord z80ToHostByteOrder(UnsignedWord value)
    {
        if constexpr (Z80ByteOrder == HostByteOrder) {
            return value;
        }

        return swapByteOrder(value);
    }

    inline constexpr UnsignedWord hostToZ80ByteOrder(UnsignedWord value)
    {
        if constexpr (Z80ByteOrder == HostByteOrder) {
            return value;
        }

        return swapByteOrder(value);
    }

}

#endif //Z80_ENDIAN_H
