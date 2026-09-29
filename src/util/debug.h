//
// Created by darren on 17/04/2021.
//

#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#include <format>
#include <iostream>

namespace Util
{
    template <typename... Args>
    void debugln(const std::format_string<Args...> format, Args&&... args)
    {
#if !defined(NDEBUG)
        std::println(std::cerr, format, std::forward<Args>(args)...);
#endif
    }

    template <typename... Args>
    void debug(const std::format_string<Args...> format, Args&&... args)
    {
#if !defined(NDEBUG)
        std::print(std::cerr, format, std::forward<Args>(args)...);
#endif
    }
}

#endif //UTIL_DEBUG_H
