#include "gpuconfig.h"
#ifdef GPU_USE_VULKAN
    #include "vulkan/GPUSurfaceVulkan.cpp"
    #include "vulkan/GPUVulkanUtils.cpp"
    #include "vulkan/ProcTable.cpp"
#endif

#ifdef GPU_USE_D3D12
    #include "d3d12/ProcTable.cpp"
#endif

#include "common/GPU.cpp"