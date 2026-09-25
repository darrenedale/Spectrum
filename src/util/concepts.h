//
// Created by Darren Edale on 17/05/2021.
//

#ifndef SPECTRUM_UTIL_CONCEPTS_H
#define SPECTRUM_UTIL_CONCEPTS_H

namespace Util
{
    template<typename T>
    concept byte_integral = std::is_integral_v<T> && 1 == sizeof(T);
}

#endif //SPECTRUM_UTIL_CONCEPTS_H
