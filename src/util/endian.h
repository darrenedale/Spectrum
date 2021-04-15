//
// Created by darren on 15/04/2021.
//

#ifndef UTIL_ENDIAN_H
#define UTIL_ENDIAN_H

#include <type_traits>

namespace Util
{
    /**
     * Convert the byte order of a 16-bit value.
     *
     * @param value
     * @return
     */
    template<typename int_t>
    constexpr inline std::enable_if_t<std::is_integral_v<int_t> && 2 == sizeof(int_t), int_t>
    swapByteOrder(int_t value)
    {
        return ((value & 0x00ff) << 8) | ((value & 0xff00) >> 8);
    }
}

#endif //UTIL_ENDIAN_H
