#pragma once
#include <type_traits>

namespace rk {
    namespace detail {
        template <size_t N, typename HT>
        struct _at {
            using type = typename HT::tail::template at<N-1>;
        };

        template <typename HT>
        struct _at<0, HT> {
            using type = typename HT::head;
        };
    }

    template <typename T, typename ...Args>
    struct typelist {
        using identity = typelist<T, Args...>;
        using head = T;
        using tail = typelist<Args...>;

        template <size_t N>
        using at = typename detail::_at<N, identity>::type;
    };

    template <typename T>
    struct typelist<T> {
        using identity = typelist<T>;
        using head = T;
        using tail = typelist<void>;

        template <size_t N>
        using at = typename detail::_at<N, identity>::type;
    };

    template <>
    struct typelist<void> {
        using head = void;
        using identity = typelist<void>;

        template <size_t N>
        using at = typename detail::_at<N, identity>::type;
    };

    namespace detail {
        template <typename T, typename TypelistType>
        struct typelist_index;

        //! Searching for type in typelist.
        template <typename T, typename U, typename ...Ts>
        struct typelist_index<T, typelist<U, Ts...>> : std::integral_constant<std::size_t, 1 + typelist_index<T, typelist<Ts...>>::value> {};

        //! Found for type in typelist.
        template <typename T, typename ...Ts>
        struct typelist_index<T, typelist<T, Ts...>> : std::integral_constant<std::size_t, 0> {};

        template <typename T>
        struct typelist_index<T, typelist<T>> : std::integral_constant<std::size_t, 0> {};

        //! Case where the type does not exist within the typelist.
        template <typename T, typename U>
        struct typelist_index<T, typelist<U>> : std::integral_constant<std::size_t, 0> {
            static_assert(std::is_same<T, U>::value, "Specified type not found within typelist!");
        };

    }

    //! Get the zero-indexed offset index of a type in a given typelist.
    template <typename T, typename U>
    constexpr size_t typelist_index_of = detail::typelist_index<T, U>::value;
}
