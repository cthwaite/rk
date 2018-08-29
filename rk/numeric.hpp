#pragma once
#include <cstdint>

namespace rk {
    static constexpr float fPI = 3.14159265358979323846f;
    //! Get the smaller of two values.
    template <typename T>
    constexpr inline T min(const T &lhs, const T &rhs)
    {
        return lhs < rhs ? lhs : rhs;
    }

    //! Get the larger of two values.
    template <typename T>
    constexpr inline T max(const T &lhs, const T &rhs)
    {
        return lhs > rhs ? lhs : rhs;
    }

    //! Clamp a value between an upper and lower bound.
    template <typename T>
    constexpr inline T clamp(const T &val, const T &min, const T &max)
    {
        return (val > max) ? max : (val < min) ? min : val;
    }

    //! Translate a value from one scale to another.
    template <typename T>
    constexpr inline T scale(T &val, T source_lower, T source_upper, T dest_lower, T dest_upper)
    {
        return (dest_upper - dest_lower) * (val - source_lower) / (source_upper - source_lower) + dest_lower;
    }

    //! Interpolate two values by a given factor.
    template <typename T>
    constexpr inline T lerp(T a, T b, float factor)
    {
        return a + ((b - a) * factor);
    }

    //! Round a 32-bit value up to the next power of two.
    inline uint32_t npot32(uint32_t value)
    {
        --value;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        return ++value;
    }

    //! Round a 64-bit value up to the next power of two.
    inline uint64_t npot64(uint64_t value)
    {
        --value;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        return ++value;
    }
}
