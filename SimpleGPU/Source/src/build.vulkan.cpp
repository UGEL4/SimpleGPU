#include "gpuconfig.h"

#ifdef GPU_USE_VULKAN
    #include "vulkan/GPUVulkanInstance.cpp"
    #define VMA_IMPLEMENTATION
    #define VMA_STATIC_VULKAN_FUNCTIONS 0
    #define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
    #include "backend/vulkan/vma/vk_mem_alloc.h"
#endif // GPU_USE_VULKAN