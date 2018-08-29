#pragma once
#include <cstdint>
#include "type_traits.hpp"

namespace rk {
    template <typename IDTag, typename ValueType>
    struct base_id_type {
        static_assert(is_integer<ValueType>(),
                      "value type of base_id_type must be an integer type!");
        using value_type = ValueType;
        using id_tag_type = IDTag;

        constexpr base_id_type() = default;
        constexpr explicit base_id_type(value_type t) :
            id{t}
        {
        }
        base_id_type(const base_id_type &) = default;
        base_id_type(base_id_type &&) = default;
        base_id_type& operator=(const base_id_type &) = default;
        base_id_type& operator=(base_id_type &&) = default;

        constexpr explicit operator value_type() const
        {
            return id;
        }

        value_type id;
    };

    template <typename IDTag, typename ValueType = uint32_t>
    struct id_type : base_id_type<IDTag, ValueType> {
        using base_type = base_id_type<IDTag, ValueType>;
        using base_type::id;
        using base_type::base_id_type;
        using typename base_type::value_type;
        using typename base_type::id_tag_type;


        id_type() = default;
        id_type(const id_type &) = default;
        id_type(id_type &&) = default;
        id_type& operator=(const id_type &) = default;
        id_type& operator=(id_type &&) = default;

        bool operator<(const id_type &other)
        {
            return id < other.id;
        }

        bool operator>(const id_type &other)
        {
            return id > other.id;
        }

        bool operator==(const id_type &other)
        {
            return id == other.id;
        }

        bool operator!=(const id_type &other)
        {
            return id != other.id;
        }

        id_type& operator++()
        {
            ++id;
            return *static_cast<id_tag_type*>(this);
        }

        id_type operator++(int)
        {
            id_type cpy(id);
            ++id;
            return cpy;
        }

        id_type& operator--()
        {
            --id;
            return getIDType();
        }

        id_type& operator--(int)
        {
            id_type cpy(id);
            --id;
            return getIDType();
        }

    private:
        id_type& getIDType()
        {
            static_assert(std::is_base_of<id_type, id_tag_type>(), "");
            return *static_cast<id_tag_type*>(this);
        }
    };
}

