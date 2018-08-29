#pragma once
#include "ext/xxhash.hpp"
#include <cstdint>


namespace rk {
//----[ XXHash ]----------------------------------------------------------------
    //! XXHash const void* convenience function.
    inline size_t xxhash(const void *ptr, size_t len, unsigned long long seed = 0)
    {
        return XXH64(ptr, len, seed);
    }

    //! XXHash byte-array convenience function.
    inline size_t xxhash(const uint8_t *ptr, size_t len, unsigned long long seed = 0)
    {
        return XXH64(reinterpret_cast<const void*>(ptr), len, seed);
    }

    //! XXHash C-string convenience function.
    inline size_t xxhash(const char *ptr, size_t len, unsigned long long seed = 0)
    {
        return XXH64(reinterpret_cast<const void*>(ptr), len, seed);
    }

    //! XXHash 32-bit finalizer.
    inline uint32_t xx_hash_int32(uint32_t h32)
    {
        h32 ^= h32 >> 15;
        h32 *= 0x85ebca77;
        h32 ^= h32 >> 13;
        h32 *= 0xc2b2ae3d;
        h32 ^= h32 >> 16;
        return h32;
    }

    //! XXHash 64-bit finalizer.
    inline uint64_t xx_hash_int64(uint64_t h64)
    {
        h64 ^= h64 >> 33;
        h64 *= 0xc2b2ae3d27d4eb4f;
        h64 ^= h64 >> 29;
        h64 *= 0x165667b19e3779f9;
        h64 ^= h64 >> 32;

        return h64;
    }

//----[ FNV32 ]-----------------------------------------------------------------

    //! FNV32 array hash.
    inline uint32_t fnv32(const void *key, const size_t len)
    {
        const uint8_t *bytes = reinterpret_cast<const uint8_t*>(key);
        uint32_t hash = 0x811c9dc5;
        for(size_t iter = 0; iter < len; ++iter)
        {
            hash = hash * 0x1000193 ^ bytes[iter];
        }
        return hash;
    }

    inline uint64_t fnv64(const void *key, const size_t len)
    {
        const uint8_t *bytes = reinterpret_cast<const uint8_t*>(key);
        uint64_t hash = 0xcbf29ce484222325;
        for(size_t iter = 0; iter < len; ++iter)
        {
            hash = hash * 0x100000001b3 ^ bytes[iter];
        }
        return hash;
    }
}

