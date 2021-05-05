//
// Created by darren on 02/05/2021.
//

#ifndef SPECTRUM_UTIL_NUMERIC_H
#define SPECTRUM_UTIL_NUMERIC_H

namespace Util
{
    /**
     * Ensure a value is rendered numerically.
     *
     * Use this when streaming to standard streams and you need char types to be output as numbers.
     *
     * @tparam T An int type that is usually rendered to output streams as a character.
     *
     * @param value The value to promote.
     *
     * @return The promoted value.
     */
    template<class T>
    inline auto asNumeric(T value) -> decltype(+value)
    {
        return +value;
    }
}

#endif //SPECTRUM_UTIL_NUMERIC_H
