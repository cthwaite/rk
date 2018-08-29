#pragma once
#include <algorithm>

namespace rk {
    /**
     * @brief   Erase-remove element(s) from a container.
     * @detail  Calls erase() on the iterator returned from a call to
     * std::remove, moving matching values to the end of the container and
     * erasing them. The passed container should have a value_type typedef.
     *
     * @tparam Container    Container type
     * @param container     Container to remove element(s) from.
     * @param match         Value of element(s) to remove.
     */
    template <typename Container>
    inline void erase_remove(Container &container, const typename Container::value_type &value)
    {
        container.erase(std::remove(container.begin(), container.end(), value), container.end());
    }

    //! Reversed range-iterator wrapper.
    namespace detail {
    template <typename Container>
    struct reversed_range_wrapper {
        using container_type = Container;
        using iterator = typename container_type::reverse_iterator;
        using const_iterator = typename container_type::const_reverse_iterator;

        reversed_range_wrapper(container_type &container) :
            container_{container}
        {
        }

        iterator begin()
        {
            return container_.rbegin();
        }

        iterator end()
        {
            return container_.rend();
        }
    private:
        container_type &container_;
    };

    /**
     * @brief Range-based iterator reverser.
     * @detail Used to wrap any container implementing rbegin and rend in a
     * range-based iteration.
     */
    template <typename C>
    inline reversed_range_wrapper<C> reverse_range(C &c)
    {
        return {c};
    }

    namespace detail{
        template <typename T>
        class base_for_range {
        public:
            using value_type = T;

            base_for_range(T l, T u) :
                l_{l},
                u_{u}
            {
            }

            struct iterator {
                iterator(T t) :
                    t_{t}
                {
                }

                iterator& operator++()
                {
                    ++t_;
                    return *this;
                }

                T operator*() const
                {
                    return t_;
                }

                bool operator!=(iterator &rhs)
                {
                    return t_ != rhs.t_;
                }
            private:
                T t_;
            };

            iterator begin() const
            {
                return l_;
            }

            iterator end() const
            {
                return u_;
            }
        private:
            const T l_,
                    u_;
        };

        template <typename T>
        class step_for_range {
        public:
            using value_type = T;

            step_for_range(T l, T u, T s) :
                l_{l},
                u_{u},
                s_{s}
            {
            }

            struct iterator {
                iterator(T t, T s = 1) :
                    t_{t},
                    s_{s}
                {
                }

                iterator& operator++()
                {
                    t_ += s_;
                    return *this;
                }

                T operator*() const
                {
                    return t_;
                }

                bool operator!=(iterator &rhs)
                {
                    return t_ < rhs.t_;
                }
            private:
                T t_,
                  s_;
            };

            iterator begin() const
            {
                return {l_, s_};
            }

            iterator end() const
            {
                return u_;
            }
        private:
            const T l_,
                    u_,
                    s_;
        };
    }

    /**
     * @brief Iterate over a numeric range.
     *
     * @param lower Lower end of range.
     * @param upper Upper end of range.
     * @return base_for_range object.
     */
    template <typename T>
    inline detail::base_for_range<T> for_range(T lower, T upper)
    {
        return {lower, upper};
    }

    /**
     * @brief Iterate over a numeric range, specifying a custom interval.
     *
     * @param lower Lower end of range.
     * @param upper Upper end of range.
     * @param step Step for each iteration.
     * @return step_for_range object.
     */
    template <typename T>
    inline detail::step_for_range<T> for_range(T lower, T upper, T step)
    {
        return {lower, upper, step};
    }
}

