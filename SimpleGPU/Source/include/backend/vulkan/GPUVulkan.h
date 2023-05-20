#pragma once
#include "api.h"

#if defined(_WIN64)
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "volk.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus extern "C" {

#define GLOBAL_VkAllocationCallbacks VK_NULL_HANDLE

	const GPUProcTable* GPUVulkanProcTable();
	const GPUSurfacesProcTable* GPUVulkanSurfacesTable();

	//instance api
	GPUInstanceID CreateInstance_Vulkan(const GPUInstanceDescriptor* pDesc);
	void FreeInstance_Vllkan(GPUInstanceID pInstance);

	//adapter
    void EnumerateAdapters_Vulkan(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount);
    VkFormat GPUFormatToVulkanFormat(EGPUFormat format);
    EGPUFormat GPUVulkanFormatToGPUFormat(VkFormat format);

	//device api
    GPUDeviceID CreateDevice_Vulkan(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc);
    void FreeDevice_Vulkan(GPUDeviceID pDevice);
    uint32_t QueryQueueCount_Vulkan(const GPUAdapterID pAdapter, const EGPUQueueType queueType);
    GPUQueueID GetQueue_Vulkan(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);

	//swapchain
    GPUSwapchainID GPUCreateSwapchain_Vulkan(GPUDeviceID pDevice, GPUSwapchainDescriptor* pDesc);
    void GPUFreeSwapchain_Vulkan(GPUSwapchainID pSwapchain);

	typedef struct GPUInstance_Vulkan
	{
		GPUInstance super;
		VkInstance pInstance;
        VkDebugUtilsMessengerEXT pDebugUtils;

		//physica device
		struct GPUAdapter_Vulkan* pAdapters;
        uint32_t adapterCount;

		//layers
		uint32_t layersCount;
		VkLayerProperties* pLayerProperties;
		const char** pLayerNames;

		//extensions
        uint32_t extensionCount;
        VkExtensionProperties* pExtensonProperties;
        const char** pExtensionNames;

		uint32_t debugUtils : 1;
	} GPUInstance_Vulkan;

	typedef struct GPUAdapter_Vulkan
	{
        GPUAdapter super;
        VkPhysicalDevice pPhysicalDevice;
        VkPhysicalDeviceProperties2KHR physicalDeviceProperties;
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures;
        VkPhysicalDeviceSubgroupProperties subgroupProperties;

        VkQueueFamilyProperties* pQueueFamilyProperties;
        uint32_t queueFamiliesCount;
        int64_t queueFamilyIndices[GPU_QUEUE_TYPE_COUNT];

		uint32_t extensionsCount;
        const char** ppExtensionsName;
        VkExtensionProperties* pExtensionProps;

		GPUAdapterDetail adapterDetail;
	} GPUAdapter_Vulkan;

	typedef struct GPUDevice_Vulkan
	{
        GPUDevice spuer;
        VkDevice pDevice;
        struct VolkDeviceTable mVkDeviceTable;
	} GPUDevice_Vulkan;

	typedef struct GPUQueue_Vulkan
	{
        const GPUQueue super;
        VkQueue pQueue;
        uint32_t queueFamilyIndex;
	} GPUQueue_Vulkan;

	typedef struct GPUSwapchain_Vulkan {
        GPUSwapchain super;
        VkSurfaceKHR pVkSurface;
        VkSwapchainKHR pVkSwapchain;
    } GPUSwapchain_Vulkan;

    typedef struct GPUTexture_Vulkan
    {
        GPUTexture super;
        VkImage pVkImage;
        union
        {
            VkDeviceMemory pVkDeviceMemory;
        };
    } GPUTexture_Vulkan;

#ifdef __cplusplus
}
#endif //__cplusplus end extern "C" }
