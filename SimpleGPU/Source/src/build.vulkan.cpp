#include "gpuconfig.h"

#ifdef GPU_USE_VULKAN
    #include "vulkan/GPUVulkanInstance.cpp"
    #define VMA_IMPLEMENTATION
    #include "backend/vulkan/vma/vk_mem_alloc.h"
#endif // GPU_USE_VULKAN