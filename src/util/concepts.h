//
// Created by Darren Edale on 17/05/2021.
//

#ifndef SPECTRUM_UTIL_CONCEPTS_H
#define SPECTRUM_UTIL_CONCEPTS_H

/**
 * Polyfill for missing concepts library in (e.g.) clang < 13
 */
namespace std
{
    template<class Derived, class Base>
    concept derived_from =
            std::is_base_of_v<Base, Derived> &&
            std::is_convertible_v<const volatile Derived*, const volatile Base*>;

    template <class From, class To>
    concept convertible_to =
        std::is_convertible_v<From, To> &&
        requires(std::add_rvalue_reference_t<From> (&f)())
        {
            static_cast<To>(f());
        };
}

#endif //SPECTRUM_UTIL_CONCEPTS_H
