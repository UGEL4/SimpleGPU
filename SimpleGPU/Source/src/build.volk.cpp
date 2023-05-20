#include "gpuconfig.h"

#ifdef GPU_USE_VULKAN
    #include "backend/vulkan/GPUVulkan.h"
    #include "vulkan/volk.c"
#endif // GPU_USE_VULKAN
