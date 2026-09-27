/*
 * Copyright (c) 2026 Darren Edale
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SPECTRUM_UTIL_ASSERT_H
#define SPECTRUM_UTIL_ASSERT_H

#if defined(NDEBUG)

#define sp_assert(EXPRESSION, MESSAGE, ...) ((void)0)

#else

/// To use this macro the translation unit where it's used must include both this header and the <print> header from the
/// standard library.
#define sp_assert(EXPRESSION, MESSAGE, ...)                         \
    if (!(EXPRESSION)) {                                            \
        std::print(stderr, "assertion {} failed: ", #EXPRESSION);   \
        std::println(stderr, (MESSAGE), ##__VA_ARGS__);             \
        std::abort();                                               \
    }

#endif

#endif // SPECTRUM_UTIL_ASSERT_H
