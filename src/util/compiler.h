//
// Created by darren on 21/05/2021.
//
// Contains macros for instructing the compiler to ignore certain warnings. Use of these macros must be restricted to
// limited and precisely targeted sections of code, and must be accompanied by a clearly documented justification
//
// Credit to Jonathan Boccara for the STRINGIFIED_PRAGMA macro
//

#ifndef SPECTRUM_UTIL_COMPILER_H
#define SPECTRUM_UTIL_COMPILER_H

#if defined(__clang__)

#define STRINGIFIED_PRAGMA(STR) _Pragma(#STR)
#define DISABLE_WARNING_PUSH _Pragma("clang diagnostic push")
#define DISABLE_WARNING_POP _Pragma("clang diagnostic pop")
#define DISABLE_WARNING(warningName) STRINGIFIED_PRAGMA(clang diagnostic ignored #warningName)

#define DISABLE_WARNING_RETURN_TYPE DISABLE_WARNING(-Wreturn-type)
#define DISABLE_WARNING_NO_RETURN_VALUE DISABLE_WARNING(-Wreturn-type)
#define DISABLE_WARNING_SWITCH DISABLE_WARNING(-Wswitch)
#define DISABLE_WARNING_UNKNOWN_PRAGMA DISABLE_WARNING(-Wunknown-pragmas)

#elif defined(__GNUC__)

#define STRINGIFIED_PRAGMA(STR) _Pragma(#STR)
#define DISABLE_WARNING_PUSH _Pragma("GCC diagnostic push")
#define DISABLE_WARNING_POP _Pragma("GCC diagnostic pop")
#define DISABLE_WARNING(warningName) STRINGIFIED_PRAGMA(GCC diagnostic ignored #warningName)

#define DISABLE_WARNING_RETURN_TYPE DISABLE_WARNING(-Wreturn-type)
#define DISABLE_WARNING_NO_RETURN_VALUE DISABLE_WARNING(-Wreturn-type)
#define DISABLE_WARNING_SWITCH DISABLE_WARNING(-Wswitch)
#define DISABLE_WARNING_UNKNOWN_PRAGMA DISABLE_WARNING(-Wunknown-pragmas)

#elif defined(_MSC_VER)

#define DISABLE_WARNING_PUSH __pragma(warning(push))
#define DISABLE_WARNING_POP __pragma(warning(pop))

// NOTE disable accepts a space-separated list of warning numbers
#define DISABLE_WARNING(warningNumbers) __pragma(warning(disable : warningNumbers))

#define DISABLE_WARNING_RETURN_TYPE DISABLE_WARNING(4033)
#define DISABLE_WARNING_NO_RETURN_VALUE DISABLE_WARNING(4715)
#define DISABLE_WARNING_SWITCH DISABLE_WARNING(4061 4062)
#define DISABLE_WARNING_UNKNOWN_PRAGMA DISABLE_WARNING(4068)

#else

#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING(warningNumber)
#define DISABLE_WARNING_RETURN_TYPE
#define DISABLE_WARNING_NO_RETURN_VALUE
#define DISABLE_WARNING_SWITCH
#define DISABLE_WARNING_UNKNOWN_PRAGMA

#endif

#endif //SPECTRUM_UTIL_COMPILER_H
