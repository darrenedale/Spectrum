//
// Created by darren on 17/04/2021.
//

#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <cstdio>

namespace Util
{
    void debug(std::format_string<Args...> format, Args&&... args)
    {
#if !defined(NDEBUG)
        std::println(std::stderr, format, std::forward<Args...>(args));
#endif
    }
}

#endif //UTIL_DEBUG_H
