#pragma once
#include <functional>
#include <string>
#include "api.h"

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

    static inline bool FormatUtil_IsDepthStencilFormat(EGPUFormat const fmt)
    {
        switch (fmt)
        {
            case GPU_FORMAT_D24_UNORM_S8_UINT:
            case GPU_FORMAT_D32_SFLOAT_S8_UINT:
            case GPU_FORMAT_D16_UNORM_S8_UINT:
            case GPU_FORMAT_D16_UNORM:
            case GPU_FORMAT_D32_SFLOAT:
                return true;
            default:
                return false;
        }
        return false;
    }

    static inline bool FormatUtil_IsDepthOnlyFormat(EGPUFormat const fmt)
    {
        switch (fmt)
        {
            case GPU_FORMAT_D16_UNORM:
            case GPU_FORMAT_D32_SFLOAT:
                return true;
            default:
                return false;
        }
        return false;
    }
}