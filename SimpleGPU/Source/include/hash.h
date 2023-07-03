#pragma once
#define XXH_INLINE_ALL
#include "xxhash.h"

#define DEFAULT_HASH_SEED_64 8053064571610612741
#define DEFAULT_HASH_SEED DEFAULT_HASH_SEED_64

inline size_t Hash64(const void* buffer, size_t size, size_t seed)
{
    return XXH64(buffer, size, seed);
}