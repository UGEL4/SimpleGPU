#pragma once
#include <cstdint>
#include "backend/vulkan/GPUVulkan.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	static const char* validationLayerName = "VK_LAYER_KHRONOS_validation";

	void VulkanUtil_SelectValidationLayers(struct GPUInstance_Vulkan* pInstance, const char** instanceLayers, uint32_t layersCount);
    void VulkanUtil_SelectInstanceExtensions(struct GPUInstance_Vulkan* pInstance, const char** instanceExtensions, uint32_t extensionCount);
    void VulkanUtil_EnableValidationLayers(struct GPUInstance_Vulkan* pInstance, const struct VkDebugUtilsMessengerCreateInfoEXT* pMessengerCreateInfo);
    VKAPI_ATTR VkBool32 VKAPI_CALL
    VulkanUtil_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    void VulkanUtil_QueryAllAdapters(struct GPUInstance_Vulkan* pInstance, const char** ppExtensions, uint32_t extensionsCount);
    void VulkanUtil_SelectQueueFamilyIndex(GPUAdapter_Vulkan* pAdapter);
    void VulkanUtil_SelectPhysicalDeviceExtensions(GPUAdapter_Vulkan* pAdapter, const char** ppExtensions, uint32_t extensionsCount);
    void VulkanUtil_EnumFormatSupport(GPUAdapter_Vulkan* pAdapter);

    uint32_t VulkanUtil_BitSizeOfBlock(EGPUFormat format);
    VkSampleCountFlagBits VulkanUtil_SampleCountToVk(EGPUSampleCount sampleCount);
    VkPrimitiveTopology VulkanUtil_PrimitiveTopologyToVk(EGPUPrimitiveTopology topology);
    VkCompareOp VulkanUtil_CompareOpToVk(EGPUCompareMode compareMode);
    VkStencilOp VulkanUtil_StencilOpToVk(EGPUStencilOp op);

    static const char* intanceWantedExtensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#if VK_KHR_device_group_creation
        VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
#endif
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    };

    static const char* deviceWantedExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
#if VK_KHR_device_group
        , VK_KHR_DEVICE_GROUP_EXTENSION_NAME
#endif
    };

    static const VkCullModeFlagBits gVkCullModeTranslator[GPU_CULL_MODE_COUNT] = {
        VK_CULL_MODE_NONE,
        VK_CULL_MODE_BACK_BIT,
        VK_CULL_MODE_FRONT_BIT
    };

    static const VkPolygonMode gVkFillModeTranslator[GPU_FILL_MODE_COUNT] = {
        VK_POLYGON_MODE_FILL,
        VK_POLYGON_MODE_LINE
    };

    static const VkFrontFace gVkFrontFaceTranslator[] = {
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FRONT_FACE_CLOCKWISE
    };

    static const VkBlendFactor gVkBlendConstantTranslator[GPU_BLEND_CONST_COUNT] = {
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_SRC_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        VK_BLEND_FACTOR_DST_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        VK_BLEND_FACTOR_SRC_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        VK_BLEND_FACTOR_DST_ALPHA,
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
        VK_BLEND_FACTOR_CONSTANT_COLOR,
        VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    };

    static const VkBlendOp gVkBlendOpTranslator[GPU_BLEND_MODE_COUNT] = {
        VK_BLEND_OP_ADD,
        VK_BLEND_OP_SUBTRACT,
        VK_BLEND_OP_REVERSE_SUBTRACT,
        VK_BLEND_OP_MIN,
        VK_BLEND_OP_MAX,
    };

    static const VkAttachmentLoadOp gVkAttachmentLoadOpTranslator[GPU_LOAD_ACTION_COUNT] = {
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_LOAD,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
    };

    static const VkAttachmentStoreOp gVkAttachmentStoreOpTranslator[GPU_STORE_ACTION_COUNT] = {
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE
    };

#ifdef __cplusplus
}
#endif // __cplusplus