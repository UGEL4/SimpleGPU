#pragma once
#define GPU_USE_VULKAN

#ifndef gpu_max
    #define gpu_max(a, b) (((a) > (b)) ? (a) : (b))
#endif // cgpu_max
#ifndef gpu_min
    #define gpu_min(a, b) (((a) < (b)) ? (a) : (b))
#endif // cgpu_max

