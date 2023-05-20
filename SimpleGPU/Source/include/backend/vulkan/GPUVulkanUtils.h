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

#ifdef __cplusplus
}
#endif // __cplusplus