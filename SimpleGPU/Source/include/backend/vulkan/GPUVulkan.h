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
    uint32_t GPUAcquireNextImage_Vulkan(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor* desc);

    //texture & texture_view api
    GPUTextureViewID GPUCreateTextureView_Vulkan(GPUDeviceID pDevice, const GPUTextureViewDescriptor* pDesc);
    void GPUFreeTextureView_Vulkan(GPUTextureViewID pTextureView);

    //shader
    GPUShaderLibraryID GPUCreateShaderLibrary_Vulkan(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc);
    void GPUFreeShaderLibrary_Vulkan(GPUShaderLibraryID pShader);

    //pipeline
    GPURenderPipelineID GPUCreateRenderPipeline_Vulkan(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc);
    void GPUFreeRenderPipeline_Vulkan(GPURenderPipelineID pPipeline);

    //rootsignature
    GPURootSignatureID GPUCreateRootSignature_Vulkan(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc);
    void GPUFreeRootSignature_Vulkan(GPURootSignatureID RS);
    void CGPUUtil_InitRSParamTables(GPURootSignature* RS, const struct GPURootSignatureDescriptor* desc);
    void GPUUtil_FreeRSParamTables(GPURootSignature* RS);

    //command
    GPUCommandPoolID GPUCreateCommandPool_Vulkan(GPUQueueID queue);
    VkCommandPool AllocateTransientCommandPool(struct GPUDevice_Vulkan* D, GPUQueueID queue);
    void GPUFreeCommandPool_Vulkan(GPUCommandPoolID pool);
    void GPUResetCommandPool_Vulkan(GPUCommandPoolID pool);
    GPUCommandBufferID GPUCreateCommandBuffer_Vulkan(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc);
    void GPUFreeCommandBuffer_Vulkan(GPUCommandBufferID cmd);

    //fence & semaphore
    GPUFenceID GPUCreateFence_Vulkan(GPUDeviceID device);
    void GPUFreeFence_Vulkan(GPUFenceID fence);
    void GPUWaitFences_Vulkan(const GPUFenceID* fences, uint32_t fenceCount);
    EGPUFenceStatus GPUQueryFenceStatus_Vulkan(GPUFenceID fence);
    GPUSemaphoreID GPUCreateSemaphore_Vulkan(GPUDeviceID device);
    void GPUFreeSemaphore_Vulkan(GPUSemaphoreID semaphore);

    typedef struct VkUtil_DescriptorPool
    {
        struct GPUDevice_Vulkan* Device;
        VkDescriptorPool pVkDescPool;
        VkDescriptorPoolCreateFlags mFlags;
    } VkUtil_DescriptorPool;

    typedef union VkDescriptorUpdateData
    {
        VkDescriptorImageInfo mImageInfo;
        VkDescriptorBufferInfo mBufferInfo;
        VkBufferView pBuferView;
    } VkDescriptorUpdateData;

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
        VkUtil_DescriptorPool* pDescriptorPool;
        struct VolkDeviceTable mVkDeviceTable;
        struct GPUVkPassTable* pPassTable;
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

    typedef struct GPUTextureView_Vulkan
    {
        GPUTextureView super;
        VkImageView pVkRTVDSVDescriptor;
        VkImageView pVkSRVDescriptor;
        VkImageView pVkUAVDescriptor;
    } GPUTextureView_vulkan;

    typedef struct GPUShaderLibrary_Vulkan
    {
        GPUShaderLibrary super;
        VkShaderModule pShader;
        struct SpvReflectShaderModule* pReflect;
    } GPUShaderLibrary_Vulkan;

    typedef struct SetLayout_Vulkan
    {
        VkDescriptorSetLayout pLayout;
        VkDescriptorUpdateTemplate pUpdateTemplate;
        uint32_t updateEntriesCount;
        VkDescriptorSet pEmptyDescSet;
    } SetLayout_Vulkan;

    typedef struct GPURootSignature_Vulkan
    {
        GPURootSignature super;
        VkPipelineLayout pPipelineLayout;
        SetLayout_Vulkan* pSetLayouts;
        VkDescriptorSetLayout* pVkSetLayouts;
        uint32_t setLayoutsCount;
        VkPushConstantRange* pPushConstantRanges;
    } GPURootSignature_Vulkan;

    typedef struct VulkanRenderPassDescriptor
    {
        EGPUFormat pColorFormat[GPU_MAX_MRT_COUNT];
        EGPULoadAction pColorLoadOps[GPU_MAX_MRT_COUNT];
        EGPUStoreAction pColorStoreOps[GPU_MAX_MRT_COUNT];
        uint32_t attachmentCount;
        EGPUFormat depthFormat;
        EGPUSampleCount sampleCount;
        EGPULoadAction depthLoadOp;
        EGPUStoreAction depthStoreOp;
        EGPULoadAction stencilLoadOp;
        EGPUStoreAction stencilStoreOp;
    } VulkanRenderPassDescriptor;

    typedef struct GPURenderPipeline_Vulkan
    {
        GPURenderPipeline super;
        VkPipeline pPipeline;
    } GPURenderPipeline_Vulkan;

    typedef struct GPUCommandPool_Vulkan
    {
        GPUCommandPool super;
        VkCommandPool pPool;
    } GPUCommandPool_Vulkan;

    typedef struct GPUCommandBuffer_Vulkan
    {
        GPUCommandBuffer super;
        VkCommandBuffer pVkCmd;
        VkPipelineLayout pLayout;
        VkRenderPass pPass;
        uint32_t type;
    } GPUCommandBuffer_Vulkan;

    typedef struct GPUFence_Vulkan
    {
        GPUFence super;
        VkFence pVkFence;
        uint32_t submitted : 1;
    } GPUFence_Vulkan;

    typedef struct GPUSemaphore_Vulkan
    {
        GPUSemaphore super;
        VkSemaphore pVkSemaphore;
        uint8_t signaled : 1;
    } GPUSemaphore_Vulkan;

#ifdef __cplusplus
}
#endif //__cplusplus end extern "C" }
