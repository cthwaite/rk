#pragma once
#include <type_traits>

namespace rk {
////[ enable_if ]///////////////////////////////////////////////////////////////
    template <bool Predicate, typename T = void>
    struct enable_if {};

    template <typename T = void>
    struct enable_if<true, T> {
        using type = T;
    };

    template <bool Predicate, typename T = void>
    struct enable_if_t = typename enable_if<Predicate, T>::type;


////[ if_then ]/////////////////////////////////////////////////////////////////
    template <bool Predicate, typename If, typename Then>
    struct if_then {
        using type = If;
    };

    template <typename If, typename Then>
    struct if_then<false, If, Then> {
        using type = Then;
    };

    template <bool Predicate, typename If, typename Then>
    using if_then_t = typename if_then<Predicate, If, Then>::type;


////[ null_t ]//////////////////////////////////////////////////////////////////
    struct null_t {
        using type = void;
    };

////[ utility traits ]//////////////////////////////////////////////////////////
    template <typename ...Args>
    struct All : std::true_type {};

    template <typename T, typename ...Args>
    struct All<T, Args...> : std::conditional<T::value,
                                              All<Args...>,
                                              std::false_type>::type {};

    template <typename ...Args>
    struct enable_if_all : std::enable_if<All<Args...>::value>::type {};

    //! Get the type T as a 'bare' type, stripping references and cv-qualifiers.
    template <typename T>
    using bare = typename std::remove_cv<typename std::remove_reference<T>::type>::type;


////[ type checking ]///////////////////////////////////////////////////////////
    //! Check if a type is an integer type.
    template <typename T>
    struct is_integer : std::conditional<std::is_integral<T>::value,
                                         std::true_type,
                                         std::false_type>::type {};
    //! bool is not considered an 'integer' type.
    template <>
    struct is_integer<bool> : std::false_type {};

    //! char is not considered an 'integer' type.
    template <>
    struct is_integer<char> : std::false_type {};

    //! Check if a type is a signed integer type.
    template <typename T>
    struct is_signed_int : All<is_integer<T>,std::is_signed<T>>::type {};

    //! Check if a type is an unsigned integer type.
    template <typename T>
    struct is_unsigned_int : All<is_integer<T>,std::is_unsigned<T>>::type {};

    //! Convenience alias for enable_if on signed int.
    template <typename T>
    using enable_if_signed_int = typename std::enable_if<is_signed_int<T>::value>::type;

    //! Convenience alias for enable_if on unsigned int.
    template <typename T>
    using enable_if_unsigned_int = typename std::enable_if<is_unsigned_int<T>::value>::type;

    //! Convenience alias for enable_if on floating point.
    template <typename T>
    using enable_if_floating_point = typename std::enable_if<std::is_floating_point<T>::value>::type;

}
