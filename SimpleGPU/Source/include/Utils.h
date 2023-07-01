#pragma once
#include <functional>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

    static uint64_t GPUNameHash(const char* name)
    {
        std::string tmp(name);
        return std::hash<std::string>{} (tmp);
    }

#ifdef __cplusplus
}
#endif