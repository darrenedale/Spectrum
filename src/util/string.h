//
// Created by darren on 17/05/2021.
//

#ifndef SPECTRUM_UTIL_STRING_H
#define SPECTRUM_UTIL_STRING_H

#include <string>
#include <algorithm>

namespace Util
{
    /**
     * Concept for string types that can be used with the in-place templates in this file.
     *
     * @tparam T The type to test against the concept.
     */
    template <typename T, typename CharT = typename T::value_type>
    concept StringType = std::derived_from<T, std::basic_string<CharT>> || std::derived_from<T, std::basic_string_view<CharT>>;

    /**
     * Concept for string types that can be used with both the in-place and copying templates in this file.
     *
     * @tparam T The type to test against the concept.
     */
    template <class T, class CharT = typename T::value_type>
    concept CopyableStringType = StringType<T, CharT> && std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>;

    /**
     * Helper to left-trim a string, in-place.
     *
     * @param str The string to trim.
     *
     * @return A reference to the string, so that calls can be used in expressions.
     */
    template<StringType string_t>
    inline string_t & trim_left(string_t & str)
    {
        str.erase(str.cbegin(), std::find_if(str.cbegin(), str.cend(), [](const auto ch) -> bool {
            return !std::isspace(ch);
        }));
        return str;
    }

    /**
     * Helper to right-trim a string, in-place.
     *
     * @param str The string to trim.
     *
     * @return A reference to the string, so that calls can be used in expressions.
     */
    template<StringType string_t>
    inline string_t & trim_right(string_t & str)
    {
        str.erase(std::find_if(str.crbegin(), str.crend(), [](const auto ch) -> bool {
            return !std::isspace(ch);
        }).base(), str.cend());
        return str;
    }

    /**
     * Helper to trim a string, in-place.
     *
     * @param str The string to trim.
     *
     * @return A reference to the string, so that calls can be used in expressions.
     */
    template<StringType string_t>
    inline string_t & trim(string_t & str)
    {
        trim_left(str);
        trim_right(str);
        return str;
    }

    /**
     * Helper to copy and left-trim a string.
     *
     * @param str The string to trim.
     *
     * @return A trimmed copy of the string.
     */
    template<CopyableStringType string_t>
    inline string_t trimmed_left(const string_t & str)
    {
        auto ret = str;
        ret.erase(str.cbegin(), std::find_if(ret.cbegin(), ret.cend(), [](const auto ch) -> bool {
            return !std::isspace(ch);
        }));
        return ret;
    }

    /**
     * Helper to right-trim a string, in-place.
     *
     * @param str The string to trim.
     *
     * @return A trimmed copy of the string.
     */
    template<CopyableStringType string_t>
    inline string_t trimmed_right(const string_t & str)
    {
        auto ret = str;
        ret.erase(std::find_if(ret.crbegin(), ret.crend(), [](const auto ch) -> bool {
            return !std::isspace(ch);
        }).base(), ret.cend());
        return ret;
    }

    /**
     * Helper to trim a string, in-place.
     *
     * @param str The string to trim.
     *
     * @return A trimmed copy of the string.
     */
    template<CopyableStringType string_t>
    inline string_t trimmed(const string_t & str)
    {
        auto ret = str;
        trim_left(ret);
        trim_right(ret);
        return ret;
    }

    /**
     * Helper to convert a string to upper-case, in-place.
     *
     * @param str The string to convert.
     *
     * @return A reference to the string, so that calls can be used in expressions.
     */
    template<StringType string_t>
    inline string_t & upper_case(string_t & str)
    {
        std::transform(str.cbegin(), str.cend(), str.begin(), ::toupper);
        return str;
    }

    /**
     * Helper to convert a string to lower-case, in-place.
     *
     * @param str The string to convert.
     *
     * @return A reference to the string, so that calls can be used in expressions.
     */
    template<StringType string_t>
    inline string_t & lower_case(string_t & str)
    {
        std::transform(str.cbegin(), str.cend(), str.begin(), ::tolower);
        return str;
    }

    /**
     * Helper to retrieve a copy of a string converted to upper-case.
     *
     * @param str The string to convert.
     */
    template<CopyableStringType string_t>
    inline string_t upper_cased(const string_t & str)
    {
        auto ret = str;
        std::transform(ret.cbegin(), ret.cend(), ret.begin(), ::toupper);
        return ret;
    }

    /**
     * Helper to retrieve a copy of a string converted to lower-case.
     *
     * @param str The string to convert.
     */
    template<CopyableStringType string_t>
    inline string_t lower_cased(const string_t & str)
    {
        auto ret = str;
        std::transform(ret.cbegin(), ret.cend(), ret.begin(), ::tolower);
        return ret;
    }
}

#endif //SPECTRUM_UTIL_STRING_H
