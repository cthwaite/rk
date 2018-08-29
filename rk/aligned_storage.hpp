#pragma once
#include <type_traits>

namespace gw {
    template <typename T, size_t S = 1>
    struct AlignedStorage {
        typedef struct {
            alignas(std::alignment_of(T)::value) uint8_t data[sizeof(T) * S];
        } type;
    };
}

