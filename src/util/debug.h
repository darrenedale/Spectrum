//
// Created by darren on 17/04/2021.
//

#ifndef UTIL_DEBUG_H
#define UTIL_DEBUG_H

#if (!defined(NDEBUG))

#include <iostream>

namespace Util
{
    /**
     * Alias the debug stream to the standard error stream.
     */
    using DebugStream = std::ostream;
    static DebugStream & debug = std::cerr;
}

#else

namespace Util
{
    /**
     * Empty class to optimise away uses of Util::debug in non-debug builds.
     */
    class DebugStream
    {
    public:
        DebugStream() = default;
        DebugStream(const DebugStream &) = delete;
        DebugStream(DebugStream &&) = delete;
        void operator=(const DebugStream &) = delete;
        void operator=(DebugStream &&) = delete;
        ~DebugStream() = default;

        /**
         * Template to fake-stream any type to the "debug" stream.
         *
         * @tparam T
         * @return
         */
        template<typename T>
        DebugStream & operator<<(const T &)
        {
            return *this;
        }
    };

    static DebugStream debug;
}

#endif

#endif //UTIL_DEBUG_H