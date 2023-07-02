#pragma once
#include <functional>
#include <string>

    static uint64_t GPUNameHash(const char* name)
    {
        std::string tmp(name);
        return std::hash<std::string>{} (tmp);
    }

namespace Utils
{
    template <typename... types>
    size_t hash_val(const types&... args)
    {
        size_t seed = 0;
        hash_val(seed, args...);
        return seed;
    }

    template <typename T, typename... types>
    void hash_val(size_t& seed, const T& firstArg, const types&... args)
    {
        hash_combine(seed, firstArg);
        hash_val(seed, args...);
    }

    template <typename T>
    void hash_val(size_t& seed, const T& val)
    {
        hash_combine(seed, val);
    }

    template <typename T>
    void hash_combine(size_t& seed, const T& val)
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}