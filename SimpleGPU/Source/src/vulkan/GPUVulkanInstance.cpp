#include "backend/vulkan/GPUVulkan.h"
#include <stdlib.h>
#include <cstring>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "GPUVulkanEXTs.h"
#include "backend/vulkan/GPUVulkanUtils.h"

class VulkanBlackboard
{
public:
    VulkanBlackboard(const GPUInstanceDescriptor* pDesc)
    {
        const GPUVulkanInstanceDescriptor* vk_desc = (const GPUVulkanInstanceDescriptor*)pDesc->pChained;

        // default
        uint32_t count = sizeof(intanceWantedExtensions) / sizeof(const char*);
        mInstanceExtensions.insert(mInstanceExtensions.end(), intanceWantedExtensions, intanceWantedExtensions + count);
        count = sizeof(deviceWantedExtensions) / sizeof(const char*);
        mDeviceExtensions.insert(mDeviceExtensions.end(), deviceWantedExtensions, deviceWantedExtensions + count);

        if (pDesc->enableDebugLayer)
        {
            mInstanceLayers.push_back(validationLayerName);
            mInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (vk_desc != VK_NULL_HANDLE)
        {
            if (vk_desc->backend != EGPUBackend::GPUBackend_Vulkan)
            {
                assert(0);
                vk_desc = VK_NULL_HANDLE;
            }

            if (vk_desc->ppInstanceLayers != VK_NULL_HANDLE && vk_desc->mInstanceLayerCount > 0)
            {
                mInstanceLayers.insert(mInstanceExtensions.end(), vk_desc->ppInstanceLayers, vk_desc->ppInstanceLayers + vk_desc->mInstanceLayerCount);
            }

            if (vk_desc->ppInstanceExtensions != VK_NULL_HANDLE && vk_desc->mInstanceExtensionCount > 0)
            {
                mInstanceExtensions.insert(mInstanceExtensions.end(), vk_desc->ppInstanceExtensions, vk_desc->ppInstanceExtensions + vk_desc->mInstanceExtensionCount);
            }
        }
    }

    const VkDebugUtilsMessengerCreateInfoEXT* pDeubgUtilsMessengerCreateInfo = VK_NULL_HANDLE;
    std::vector<const char*> mInstanceLayers;
    std::vector<const char*> mInstanceExtensions;
    std::vector<const char*> mDeviceExtensions;
};

const GPUProcTable vkTable = {
    .CreateInstance       = &CreateInstance_Vulkan,
    .FreeInstance         = &FreeInstance_Vllkan,
    .EnumerateAdapters    = &EnumerateAdapters_Vulkan,
    .CreateDevice         = &CreateDevice_Vulkan,
    .FreeDevice           = &FreeDevice_Vulkan,
    .GetQueue             = &GetQueue_Vulkan,
    .SubmitQueue          = &GPUSubmitQueue_Vulkan,
    .WaitQueueIdle        = &GPUWaitQueueIdle_Vulkan,
    .QueuePresent         = &GPUQueuePresent_Vulkan,
    .CreateSwapchain      = &GPUCreateSwapchain_Vulkan,
    .FreeSwapchain        = &GPUFreeSwapchain_Vulkan,
    .AcquireNextImage     = &GPUAcquireNextImage_Vulkan,
    .CreateTextureView    = &GPUCreateTextureView_Vulkan,
    .FreeTextureView      = &GPUFreeTextureView_Vulkan,
    .CreateShaderLibrary  = &GPUCreateShaderLibrary_Vulkan,
    .FreeShaderLibrary    = &GPUFreeShaderLibrary_Vulkan,
    .CreateRenderPipeline = &GPUCreateRenderPipeline_Vulkan,
    .FreeRenderPipeline   = &GPUFreeRenderPipeline_Vulkan,
    .CreateRootSignature  = &GPUCreateRootSignature_Vulkan,
    .FreeRootSignature    = &GPUFreeRootSignature_Vulkan,
    .CreateCommandPool    = &GPUCreateCommandPool_Vulkan,
    .FreeCommandPool      = &GPUFreeCommandPool_Vulkan,
    .ResetCommandPool     = &GPUResetCommandPool_Vulkan,
    .CreateCommandBuffer  = &GPUCreateCommandBuffer_Vulkan,
    .FreeCommandBuffer    = &GPUFreeCommandBuffer_Vulkan,
    .CmdBegin             = &GPUCmdBegin_Vulkan,
    .CmdEnd               = &GPUCmdEnd_Vulkan,
    .CmdResourceBarrier   = &GPUCmdResourceBarrier_Vulkan,
    .CreateFence          = &GPUCreateFence_Vulkan,
    .FreeFence            = &GPUFreeFence_Vulkan,
    .WaitFences           = &GPUWaitFences_Vulkan,
    .QueryFenceStatus     = &GPUQueryFenceStatus_Vulkan,
    .CreateSemaphore      = &GPUCreateSemaphore_Vulkan,
    .FreeSemaphore        = &GPUFreeSemaphore_Vulkan,
    .CmdBeginRenderPass        = &GPUCmdBeginRenderPass_Vulkan,
    .CmdEndRenderPass          = &GPUCmdEndRenderPass_Vulkan,
    .RenderEncoderSetViewport  = &GPURenderEncoderSetViewport_Vulkan,
    .RenderEncoderSetScissor   = &GPURenderEncoderSetScissor_Vulkan,
    .RenderEncoderBindPipeline = &GPURenderEncoderBindPipeline_Vulkan,
    .RenderEncoderDraw         = &GPURenderEncoderDraw_Vulkan
};
const GPUProcTable* GPUVulkanProcTable()
{
    return &vkTable;
}

struct GPUCachedRenderPass
{
    VkRenderPass pPass;
    size_t timestamp;
};

struct VulkanRenderPassDescriptorHasher {
    size_t operator()(const VulkanRenderPassDescriptor& a) const
    {
        return hash_val(&a);
    }

    template <typename... types>
    size_t hash_val(const types&... args) const
    {
        size_t seed = 0;
        hash_val(seed, args...);
        return seed;
    }

    template <typename T, typename... types>
    void hash_val(size_t& seed, const T& firstArg, const types&... args) const
    {
        hash_combine(seed, firstArg);
        hash_val(seed, args...);
    }

    template <typename T>
    void hash_val(size_t& seed, const T& val) const
    {
        hash_combine(seed, val);
    }

    template <typename T>
    void hash_combine(size_t& seed, const T& val) const
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
};

bool operator==(const VulkanRenderPassDescriptor& a, const VulkanRenderPassDescriptor& b)
{
    if (a.attachmentCount != b.attachmentCount) return false;
    return std::memcmp(&a, &b, sizeof(VulkanRenderPassDescriptor)) == 0;
}

struct GPUCachedFrameBuffer
{
    VkFramebuffer pBuffer;
};

struct VulkanFramebufferDesriptorHasher {
    size_t operator()(const VulkanFramebufferDesriptor& a) const
    {
        return hash_val(&a);
    }

    template <typename... types>
    size_t hash_val(const types&... args) const
    {
        size_t seed = 0;
        hash_val(seed, args...);
        return seed;
    }

    template <typename T, typename... types>
    void hash_val(size_t& seed, const T& firstArg, const types&... args) const
    {
        hash_combine(seed, firstArg);
        hash_val(seed, args...);
    }

    template <typename T>
    void hash_val(size_t& seed, const T& val) const
    {
        hash_combine(seed, val);
    }

    template <typename T>
    void hash_combine(size_t& seed, const T& val) const
    {
        seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
};

bool operator==(const VulkanFramebufferDesriptor& a, const VulkanFramebufferDesriptor& b)
{
    if (a.attachmentCount != b.attachmentCount) return false;
    return std::memcmp(&a, &b, sizeof(VulkanFramebufferDesriptor)) == 0;
}

struct GPUVkPassTable
{
    std::unordered_map<VulkanRenderPassDescriptor, GPUCachedRenderPass, VulkanRenderPassDescriptorHasher> cached_renderpasses;
    std::unordered_map<VulkanFramebufferDesriptor, GPUCachedFrameBuffer, VulkanFramebufferDesriptorHasher> cached_framebuffers;
};

GPUInstanceID CreateInstance_Vulkan(const GPUInstanceDescriptor* pDesc)
{
    VulkanBlackboard blackBoard(pDesc);

    GPUInstance_Vulkan* I = (GPUInstance_Vulkan*)malloc(sizeof(GPUInstance_Vulkan));
    ::memset(I, 0, sizeof(GPUInstance_Vulkan));

    VkResult result = volkInitialize();
    if (result != VK_SUCCESS)
    {
        assert(0);
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "GPU";
    appInfo.apiVersion         = VK_API_VERSION_1_3;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);

    VulkanUtil_SelectValidationLayers(I, blackBoard.mInstanceLayers.data(), (uint32_t)blackBoard.mInstanceLayers.size());
    VulkanUtil_SelectInstanceExtensions(I, blackBoard.mInstanceExtensions.data(), (uint32_t)blackBoard.mInstanceExtensions.size());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = (uint32_t)blackBoard.mInstanceLayers.size();
    createInfo.ppEnabledLayerNames     = blackBoard.mInstanceLayers.data();
    createInfo.enabledExtensionCount   = (uint32_t)blackBoard.mInstanceExtensions.size();
    createInfo.ppEnabledExtensionNames = blackBoard.mInstanceExtensions.data();

    if (pDesc->enableValidation)
    {
        if (!pDesc->enableDebugLayer)
        {
            assert(0);
        }

        VkValidationFeaturesEXT validationFeatureExt{};
        validationFeatureExt.sType                    = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        VkValidationFeatureEnableEXT enableFeatures[] = {
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
            VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
        };
        validationFeatureExt.pEnabledValidationFeatures    = enableFeatures;
        validationFeatureExt.enabledValidationFeatureCount = (uint32_t)(sizeof(enableFeatures) / sizeof(VkValidationFeatureEnableEXT));
        createInfo.pNext                                   = &validationFeatureExt;
    }

    result = vkCreateInstance(&createInfo, GLOBAL_VkAllocationCallbacks, &I->pInstance);
    if (result != VK_SUCCESS)
    {
        assert(0);
    }

    volkLoadInstance(I->pInstance);

    // Adapters
    VulkanUtil_QueryAllAdapters(I, blackBoard.mDeviceExtensions.data(), (uint32_t)blackBoard.mDeviceExtensions.size());
    std::sort(I->pAdapters, I->pAdapters + I->adapterCount, [](const GPUAdapter_Vulkan& a, const GPUAdapter_Vulkan& b) {
        const uint32_t orders[] = {
            4, 1, 0, 2, 3
        };
        return orders[a.physicalDeviceProperties.properties.deviceType] < orders[b.physicalDeviceProperties.properties.deviceType];
    });

    if (pDesc->enableDebugLayer)
    {
        I->debugUtils = 1; // TODO:
        VulkanUtil_EnableValidationLayers(I, blackBoard.pDeubgUtilsMessengerCreateInfo);
    }

    return &(I->super);
}

void FreeInstance_Vllkan(GPUInstanceID pInstance)
{
    GPUInstance_Vulkan* p = (GPUInstance_Vulkan*)pInstance;

    if (p->pDebugUtils)
    {
        assert(vkDestroyDebugUtilsMessengerEXT && "load vkDestroyDebugUtilsMessengerEXT failed!");
        vkDestroyDebugUtilsMessengerEXT(p->pInstance, p->pDebugUtils, GLOBAL_VkAllocationCallbacks);
    }

    vkDestroyInstance(p->pInstance, GLOBAL_VkAllocationCallbacks);

    GPU_SAFE_FREE(p->pLayerProperties);
    GPU_SAFE_FREE(p->pLayerNames);

    GPU_SAFE_FREE(p->pExtensonProperties);
    GPU_SAFE_FREE(p->pExtensionNames);

    GPU_SAFE_FREE(p);
}

void EnumerateAdapters_Vulkan(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount)
{
    const GPUInstance_Vulkan* pVkInstance = (const GPUInstance_Vulkan*)pInstance;
    *adapterCount                         = pVkInstance->adapterCount;
    if (ppAdapters != VK_NULL_HANDLE)
    {
        for (uint32_t i = 0; i < *adapterCount; i++)
        {
            ppAdapters[i] = &pVkInstance->pAdapters[i].super;
        }
    }
}

VkFormat GPUFormatToVulkanFormat(EGPUFormat format)
{
    switch (format)
    {
        case EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case EGPUFormat::GPU_FORMAT_B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case EGPUFormat::GPU_FORMAT_R8G8BA8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case EGPUFormat::GPU_FORMAT_R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
    }
    return VK_FORMAT_UNDEFINED;
}

EGPUFormat GPUVulkanFormatToGPUFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_B8G8R8A8_UNORM:
            return EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return EGPUFormat::GPU_FORMAT_B8G8R8A8_SRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return EGPUFormat::GPU_FORMAT_R8G8BA8_UNORM;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return EGPUFormat::GPU_FORMAT_R8G8B8A8_SRGB;
    }
    return EGPUFormat::GPU_FORMT_COUNT;
}

const float queuePriorities[] = {
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, //
};
GPUDeviceID CreateDevice_Vulkan(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc)
{
    GPUInstance_Vulkan* pVkInstance = (GPUInstance_Vulkan*)pAdapter->pInstance;
    GPUDevice_Vulkan* pDevice       = (GPUDevice_Vulkan*)malloc(sizeof(GPUDevice_Vulkan));
    GPUAdapter_Vulkan* pVkAdapter   = (GPUAdapter_Vulkan*)pAdapter;

    *const_cast<GPUAdapterID*>(&pDevice->spuer.pAdapter) = pAdapter;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(pDesc->queueGroupCount);
    for (uint32_t i = 0; i < pDesc->queueGroupCount; i++)
    {
        GPUQueueGroupDescriptor& descriptor = pDesc->pQueueGroup[i];
        VkDeviceQueueCreateInfo& info       = queueCreateInfos[i];
        info.sType                          = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueCount                     = descriptor.queueCount;
        info.queueFamilyIndex               = (uint32_t)pVkAdapter->queueFamilyIndices[descriptor.queueType];
        info.pQueuePriorities               = queuePriorities;

        assert(QueryQueueCount_Vulkan(pAdapter, descriptor.queueType) >= descriptor.queueCount);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                = &pVkAdapter->physicalDeviceFeatures;
    createInfo.queueCreateInfoCount = pDesc->queueGroupCount;
    createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    createInfo.pEnabledFeatures     = VK_NULL_HANDLE;
    // layer & extension
    createInfo.enabledExtensionCount   = pVkAdapter->extensionsCount;
    createInfo.ppEnabledExtensionNames = pVkAdapter->ppExtensionsName;

    VkResult result = vkCreateDevice(pVkAdapter->pPhysicalDevice, &createInfo, GLOBAL_VkAllocationCallbacks, &pDevice->pDevice);
    if (result != VK_SUCCESS)
    {
        assert(0);
    }

    volkLoadDeviceTable(&pDevice->mVkDeviceTable, pDevice->pDevice);
    assert(pDevice->mVkDeviceTable.vkCreateSwapchainKHR);

    // pipeline cache

    //descriptor pool
    pDevice->pDescriptorPool = (VkUtil_DescriptorPool*)malloc(sizeof(VkUtil_DescriptorPool));
    memset(pDevice->pDescriptorPool, 0, sizeof(VkUtil_DescriptorPool));
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE;
    poolInfo.pPoolSizes    = gDescriptorPoolSizes;
    poolInfo.maxSets       = 8192;
    result = pDevice->mVkDeviceTable.vkCreateDescriptorPool(pDevice->pDevice, &poolInfo, GLOBAL_VkAllocationCallbacks, &pDevice->pDescriptorPool->pVkDescPool);
    assert(result == VK_SUCCESS);
    pDevice->pDescriptorPool->Device = pDevice;
    pDevice->pDescriptorPool->mFlags = poolInfo.flags;

    //pass table
    void* ptr           = calloc(1, sizeof(GPUVkPassTable));
    pDevice->pPassTable = new (ptr) GPUVkPassTable();

    return &(pDevice->spuer);
}

void FreeDevice_Vulkan(GPUDeviceID pDevice)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;

    for (auto& iter : pVkDevice->pPassTable->cached_renderpasses)
    {
        pVkDevice->mVkDeviceTable.vkDestroyRenderPass(pVkDevice->pDevice, iter.second.pPass, GLOBAL_VkAllocationCallbacks);
    }

    pVkDevice->mVkDeviceTable.vkDestroyDescriptorPool(pVkDevice->pDevice, pVkDevice->pDescriptorPool->pVkDescPool, GLOBAL_VkAllocationCallbacks);
    vkDestroyDevice(pVkDevice->pDevice, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(pVkDevice->pPassTable);
    GPU_SAFE_FREE(pVkDevice->pDescriptorPool);
    GPU_SAFE_FREE(pVkDevice);
}

VkRenderPass VulkanUtil_RenderPassTableTryFind(struct GPUVkPassTable* table, const struct VulkanRenderPassDescriptor* desc)
{
    auto iter = table->cached_renderpasses.find(*desc);
    if (iter != table->cached_renderpasses.end())
    {
        //
        return iter->second.pPass;
    }
    return VK_NULL_HANDLE;
}

void VulkanUtil_RenderPassTableAdd(struct GPUVkPassTable* table, const struct VulkanRenderPassDescriptor* desc, VkRenderPass pass)
{
    const auto& iter = table->cached_renderpasses.find(*desc);
    if (iter != table->cached_renderpasses.end())
    {
       //"Vulkan Pass with this desc already exists!";
    }
    // TODO: Add timestamp
    GPUCachedRenderPass new_pass      = { pass, 0 };
    table->cached_renderpasses[*desc] = new_pass;
}

VkFramebuffer VulkanUtil_FrameBufferTableTryFind(struct GPUVkPassTable* table, const struct VulkanFramebufferDesriptor* desc)
{
    auto iter = table->cached_framebuffers.find(*desc);
    if (iter != table->cached_framebuffers.end())
    {
        return iter->second.pBuffer;
    }
    return VK_NULL_HANDLE;
}

void VulkanUtil_FrameBufferTableAdd(struct GPUVkPassTable* table, const struct VulkanFramebufferDesriptor* desc, VkFramebuffer framebuffer)
{
    const auto& iter = table->cached_framebuffers.find(*desc);
    if (iter != table->cached_framebuffers.end())
    {
    }
    GPUCachedFrameBuffer new_framebuffer = { framebuffer};
    table->cached_framebuffers[*desc]    = new_framebuffer;
}

uint32_t QueryQueueCount_Vulkan(const GPUAdapterID pAdapter, const EGPUQueueType queueType)
{
    const GPUAdapter_Vulkan* ptr = (const GPUAdapter_Vulkan*)pAdapter;
    uint32_t count               = 0;
    switch (queueType)
    {
        case EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS: {
            for (uint32_t i = 0; i < ptr->queueFamiliesCount; i++)
            {
                VkQueueFamilyProperties& props = ptr->pQueueFamilyProperties[i];
                if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    count += props.queueCount;
                }
            }
        }
        break;
        case EGPUQueueType::GPU_QUEUE_TYPE_COMPUTE: {
            for (uint32_t i = 0; i < ptr->queueFamiliesCount; i++)
            {
                VkQueueFamilyProperties& props = ptr->pQueueFamilyProperties[i];
                if (props.queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    if (!(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                    {
                        count += props.queueCount;
                    }
                }
            }
        }
        break;
        case EGPUQueueType::GPU_QUEUE_TYPE_TRANSFER: {
            for (uint32_t i = 0; i < ptr->queueFamiliesCount; i++)
            {
                VkQueueFamilyProperties& props = ptr->pQueueFamilyProperties[i];
                if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (!(props.queueFlags & VK_QUEUE_COMPUTE_BIT))
                    {
                        if (!(props.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                        {
                            count += props.queueCount;
                        }
                    }
                }
            }
        }
        break;
        default:
            assert(0);
    }
    return count;
}

GPUQueueID GetQueue_Vulkan(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex)
{
    GPUAdapter_Vulkan* pVkAdapter = (GPUAdapter_Vulkan*)pDevice->pAdapter;
    GPUDevice_Vulkan* pVkDevice   = (GPUDevice_Vulkan*)pDevice;
    GPUQueue_Vulkan* pVkQueue     = (GPUQueue_Vulkan*)malloc(sizeof(GPUQueue_Vulkan));
    GPUQueue tmpQueue             = {
                    .pDevice    = pDevice,
                    .queueType  = queueType,
                    .queueIndex = queueIndex
    };
    *(GPUQueue*)&pVkQueue->super = tmpQueue;

    pVkDevice->mVkDeviceTable.vkGetDeviceQueue(pVkDevice->pDevice, (uint32_t)pVkAdapter->queueFamilyIndices[queueType], queueIndex, &pVkQueue->pQueue);
    pVkQueue->queueFamilyIndex = (uint32_t)pVkAdapter->queueFamilyIndices[queueType];

    return &pVkQueue->super;
}

void GPUSubmitQueue_Vulkan(GPUQueueID queue, const struct GPUQueueSubmitDescriptor* desc)
{
    GPUQueue_Vulkan* Q  = (GPUQueue_Vulkan*)queue;
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)queue->pDevice;
    GPUFence_Vulkan* F  = (GPUFence_Vulkan*)desc->signal_fence;
    uint32_t cmdCount   = desc->cmds_count;
    GPUCommandBuffer_Vulkan** vkCmds = (GPUCommandBuffer_Vulkan**)desc->cmds;
    DECLEAR_ZERO_VAL(VkCommandBuffer, cmds, cmdCount);
    for (uint32_t i = 0; i < cmdCount; i++)
    {
        cmds[i] = vkCmds[i]->pVkCmd;
    }

    GPUSemaphore_Vulkan** vkSemaphores = (GPUSemaphore_Vulkan**)desc->wait_semaphores;
    DECLEAR_ZERO_VAL(VkSemaphore, ppWaitSemaphore, desc->wait_semaphore_count + 1);
    uint32_t waitCount = 0;
    for (uint32_t i = 0; i < desc->wait_semaphore_count; i++)
    {
        if (vkSemaphores[i]->signaled)
        {
            ppWaitSemaphore[waitCount] = vkSemaphores[i]->pVkSemaphore;
            vkSemaphores[i]->signaled  = false;
            waitCount++;
        }
    }

    GPUSemaphore_Vulkan** vkSignalSemaphores = (GPUSemaphore_Vulkan**)desc->signal_semaphores;
    DECLEAR_ZERO_VAL(VkSemaphore, ppSignalSemaphore, desc->signal_semaphore_count + 1);
    uint32_t signalCount = 0;
    for (uint32_t i = 0; i < desc->signal_semaphore_count; i++)
    {
        if (!vkSignalSemaphores[i]->signaled)
        {
            ppSignalSemaphore[signalCount] = vkSignalSemaphores[i]->pVkSemaphore;
            vkSemaphores[i]->signaled    = true;
            signalCount++;
        }
    }

    VkSubmitInfo info{};
    info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.waitSemaphoreCount   = waitCount;
    info.pWaitSemaphores      = ppSignalSemaphore;
    info.pWaitDstStageMask    = 0;
    info.commandBufferCount   = cmdCount;
    info.pCommandBuffers      = cmds;
    info.signalSemaphoreCount = signalCount;
    info.pSignalSemaphores    = ppWaitSemaphore;

    VkResult rs = D->mVkDeviceTable.vkQueueSubmit(Q->pQueue, 1, &info, F ? F->pVkFence : VK_NULL_HANDLE);
    if (rs != VK_SUCCESS)
    {
        if (rs == VK_ERROR_DEVICE_LOST)
        {
        }
        else
        {
            assert(0);
        }
    }
    if (F) F->submitted = true;
}

void GPUWaitQueueIdle_Vulkan(GPUQueueID queue)
{
    GPUQueue_Vulkan* Q  = (GPUQueue_Vulkan*)queue;
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)queue->pDevice;
    D->mVkDeviceTable.vkQueueWaitIdle(Q->pQueue);
}

void GPUQueuePresent_Vulkan(GPUQueueID queue, const struct GPUQueuePresentDescriptor* desc)
{
    GPUQueue_Vulkan* Q     = (GPUQueue_Vulkan*)queue;
    GPUDevice_Vulkan* D    = (GPUDevice_Vulkan*)queue->pDevice;
    GPUSwapchain_Vulkan* S = (GPUSwapchain_Vulkan*)desc->swapchain;

    if (S)
    {
        uint32_t waitCount = 0;
        DECLEAR_ZERO_VAL(VkSemaphore, ppSemaphores, desc->wait_semaphore_count + 1);
        GPUSemaphore_Vulkan** vkSemaphores = (GPUSemaphore_Vulkan**)desc->wait_semaphores;
        for (uint32_t i = 0; i < desc->wait_semaphore_count; i++)
        {
            if (vkSemaphores[i]->signaled)
            {
                ppSemaphores[waitCount]   = vkSemaphores[i]->pVkSemaphore;
                vkSemaphores[i]->signaled = false;
                waitCount++;
            }
        }

        uint32_t presentIndex = desc->index;
        VkPresentInfoKHR info{};
        info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = waitCount;
        info.pWaitSemaphores    = ppSemaphores;
        info.swapchainCount     = 1;
        info.pSwapchains        = &S->pVkSwapchain;
        info.pImageIndices      = &presentIndex;
        info.pResults           = VK_NULL_HANDLE;

        VkResult rs = D->mVkDeviceTable.vkQueuePresentKHR(Q->pQueue, &info);
        if (rs != VK_SUCCESS && rs != VK_SUBOPTIMAL_KHR &&
            rs != VK_ERROR_OUT_OF_DATE_KHR)
        {
            assert(0 && "Present failed!");
        }
    }
}

GPUSwapchainID GPUCreateSwapchain_Vulkan(GPUDeviceID pDevice, GPUSwapchainDescriptor* pDesc)
{
    GPUAdapter_Vulkan* pVkAdapter = (GPUAdapter_Vulkan*)pDevice->pAdapter;
    GPUQueue_Vulkan* pVkQueue     = (GPUQueue_Vulkan*)pDesc->ppPresentQueues[0];
    GPUDevice_Vulkan* pVkDevice   = (GPUDevice_Vulkan*)pDevice;

    VkSurfaceKHR pVkSurface = (VkSurfaceKHR)pDesc->pSurface;

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pVkAdapter->pPhysicalDevice, pVkSurface, &capabilities);

    // format
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pVkAdapter->pPhysicalDevice, pVkSurface, &formatCount, VK_NULL_HANDLE);
    DECLEAR_ZERO_VAL(VkSurfaceFormatKHR, surfaceFormats, formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pVkAdapter->pPhysicalDevice, pVkSurface, &formatCount, surfaceFormats);
    VkSurfaceFormatKHR surfaceFormat{};
    surfaceFormat.format = VK_FORMAT_UNDEFINED;
    // Only undefined format support found, force use B8G8R8A8
    if ((1 == formatCount) && (VK_FORMAT_UNDEFINED == surfaceFormats[0].format))
    {
        surfaceFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
        surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        VkFormat requestFormat            = GPUFormatToVulkanFormat(pDesc->format);
        VkColorSpaceKHR requestColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        for (uint32_t i = 0; i < formatCount; i++)
        {
            if (requestFormat == surfaceFormats[i].format && requestColorSpace == surfaceFormats[i].colorSpace)
            {
                surfaceFormat.format     = requestFormat;
                surfaceFormat.colorSpace = requestColorSpace;
                break;
            }
        }
    }

    // present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    uint32_t presentModeCount    = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pVkAdapter->pPhysicalDevice, pVkSurface, &presentModeCount, VK_NULL_HANDLE);
    DECLEAR_ZERO_VAL(VkPresentModeKHR, presentModes, presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(pVkAdapter->pPhysicalDevice, pVkSurface, &presentModeCount, presentModes);
    // Select Preferred Present Mode
    VkPresentModeKHR preferredModeList[] = {
        VK_PRESENT_MODE_IMMEDIATE_KHR,    // normal
        VK_PRESENT_MODE_MAILBOX_KHR,      // low latency
        VK_PRESENT_MODE_FIFO_RELAXED_KHR, // minimize stuttering
        VK_PRESENT_MODE_FIFO_KHR          // low power consumption
    };
    const uint32_t preferredModeCount = sizeof(preferredModeList) / sizeof(VkPresentModeKHR);
    uint32_t preferredModeStartIndex  = pDesc->enableVSync ? 1 : 0;
    for (uint32_t j = preferredModeStartIndex; j < preferredModeCount; ++j)
    {
        VkPresentModeKHR mode = preferredModeList[j];
        uint32_t i            = 0;
        for (i = 0; i < presentModeCount; ++i)
        {
            if (presentModes[i] == mode) break;
        }
        if (i < presentModeCount)
        {
            presentMode = mode;
            break;
        }
    }

    auto clamp = [](uint32_t num, uint32_t min, uint32_t max) {
        if (num < min) return min;
        if (num > max) return max;
        return num;
    };
    VkExtent2D extent;
    extent.width  = clamp(pDesc->width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = clamp(pDesc->height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    uint32_t presentQueueFamilyIndex = -1;
    VkBool32 presentSupport          = false;
    VkResult res                     = vkGetPhysicalDeviceSurfaceSupportKHR(pVkAdapter->pPhysicalDevice, pVkQueue->queueFamilyIndex, pVkSurface, &presentSupport);
    if (res == VK_SUCCESS && presentSupport)
    {
        presentQueueFamilyIndex = pVkQueue->queueFamilyIndex;
    }
    else
    {
        uint32_t queueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pVkAdapter->pPhysicalDevice, &queueFamilyPropertyCount, VK_NULL_HANDLE);
        DECLEAR_ZERO_VAL(VkQueueFamilyProperties, props, queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pVkAdapter->pPhysicalDevice, &queueFamilyPropertyCount, props);
        // Check if hardware provides dedicated present queue
        if (queueFamilyPropertyCount)
        {
            for (uint32_t index = 0; index < queueFamilyPropertyCount; ++index)
            {
                VkBool32 supportPresent = VK_FALSE;
                VkResult res            = vkGetPhysicalDeviceSurfaceSupportKHR(pVkAdapter->pPhysicalDevice, index, pVkSurface, &supportPresent);
                if ((VK_SUCCESS == res) && (VK_TRUE == supportPresent) && pVkQueue->queueFamilyIndex != index)
                {
                    presentQueueFamilyIndex = index;
                    break;
                }
            }
            // If there is no dedicated present queue, just find the first available queue which supports
            // present
            if (presentQueueFamilyIndex == -1)
            {
                for (uint32_t index = 0; index < queueFamilyPropertyCount; ++index)
                {
                    VkBool32 supportPresent = VK_FALSE;
                    VkResult res            = vkGetPhysicalDeviceSurfaceSupportKHR(pVkAdapter->pPhysicalDevice, index, pVkSurface, &supportPresent);
                    if ((VK_SUCCESS == res) && (VK_TRUE == supportPresent))
                    {
                        presentQueueFamilyIndex = index;
                        break;
                    }
                    else
                    {
                        // No present queue family available. Something goes wrong.
                        assert(0);
                    }
                }
            }
        }
    }

    VkSurfaceTransformFlagBitsKHR preTransform;
    // #TODO: Add more if necessary but identity should be enough for now
    if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = capabilities.currentTransform;
    }

    const VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    };
    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
    uint32_t compositeAlphaFlagsCount          = sizeof(compositeAlphaFlags) / sizeof(VkCompositeAlphaFlagBitsKHR);
    for (uint32_t i = 0; i < compositeAlphaFlagsCount; i++)
    {
        if (capabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
        {
            compositeAlpha = compositeAlphaFlags[i];
            break;
        }
    }
    assert(compositeAlpha != VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface               = pVkSurface;
    createInfo.minImageCount         = pDesc->imageCount;
    createInfo.imageFormat           = surfaceFormat.format;
    createInfo.imageColorSpace       = surfaceFormat.colorSpace;
    createInfo.imageExtent           = extent;
    createInfo.imageArrayLayers      = 1;
    createInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices   = &presentQueueFamilyIndex;
    createInfo.preTransform          = preTransform;
    createInfo.compositeAlpha        = compositeAlpha;
    createInfo.presentMode           = presentMode;
    createInfo.clipped               = VK_TRUE;

    VkSwapchainKHR pVkSwapchain;
    VkResult result = pVkDevice->mVkDeviceTable.vkCreateSwapchainKHR(pVkDevice->pDevice, &createInfo, GLOBAL_VkAllocationCallbacks, &pVkSwapchain);
    if (result != VK_SUCCESS)
    {
        assert(0);
    }

    uint32_t imageCount = 0;
    pVkDevice->mVkDeviceTable.vkGetSwapchainImagesKHR(pVkDevice->pDevice, pVkSwapchain, &imageCount, VK_NULL_HANDLE);
    /*
        GPUSwapchain_Vulkan
        {
            GPUSwapchain
            {
                GPUDeviceID pDevice;
                const GPUTextureID* ppBackBuffers;
                uint32_t backBuffersCount;
            };
            VkSurfaceKHR pVkSurface;
            VkSwapchainKHR pVkSwapchain;
        };
    */
    // mem:GPUSwapchain_Vulkan + mem:(swapchain image) + mem:(GPUSwapchain.ppBackBuffers)
    uint32_t size                      = sizeof(GPUSwapchain_Vulkan) + imageCount * sizeof(GPUTexture_Vulkan) + imageCount * sizeof(GPUTextureID*);
    GPUSwapchain_Vulkan* pSwapchain    = (GPUSwapchain_Vulkan*)malloc(size);
    pSwapchain->pVkSwapchain           = pVkSwapchain;
    pSwapchain->pVkSurface             = pVkSurface;
    pSwapchain->super.backBuffersCount = imageCount;
    DECLEAR_ZERO_VAL(VkImage, images, imageCount);
    pVkDevice->mVkDeviceTable.vkGetSwapchainImagesKHR(pVkDevice->pDevice, pVkSwapchain, &imageCount, images);

    GPUTexture_Vulkan* pTex = (GPUTexture_Vulkan*)(pSwapchain + 1);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        pTex[i].pVkImage                = images[i];
        pTex[i].super.isCube            = false;
        pTex[i].super.arraySizeMinusOne = 0;
        pTex[i].super.pDevice           = &pVkDevice->spuer;
        pTex[i].super.sampleCount       = GPU_SAMPLE_COUNT_1; // TODO: ?
        pTex[i].super.format            = GPUVulkanFormatToGPUFormat(surfaceFormat.format);
        pTex[i].super.aspectMask        = VulkanUtil_DeterminAspectMask((VkFormat)pTex[i].super.format, false);
        pTex[i].super.depth     = 1;
        pTex[i].super.width     = extent.width;
        pTex[i].super.height    = extent.height;
        pTex[i].super.mipLevels = 1;
        // pTex[i].super.node_index = CGPU_SINGLE_GPU_NODE_INDEX;
        pTex[i].super.ownsImage    = false;
        pTex[i].super.nativeHandle = pTex[i].pVkImage;
    }

    GPUTextureID* Vs = (GPUTextureID*)(pTex + imageCount);
    for (uint32_t i = 0; i < imageCount; i++)
    {
        Vs[i] = &pTex[i].super;
    }
    pSwapchain->super.ppBackBuffers = Vs;

    return &pSwapchain->super;
}

void GPUFreeSwapchain_Vulkan(GPUSwapchainID pSwapchain)
{
    GPUSwapchain_Vulkan* p    = (GPUSwapchain_Vulkan*)pSwapchain;
    GPUDevice_Vulkan* pDevice = (GPUDevice_Vulkan*)p->super.pDevice;
    pDevice->mVkDeviceTable.vkDestroySwapchainKHR(pDevice->pDevice, p->pVkSwapchain, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(p);
}

uint32_t GPUAcquireNextImage_Vulkan(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor* desc)
{
    GPUSwapchain_Vulkan* S         = (GPUSwapchain_Vulkan*)swapchain;
    GPUSemaphore_Vulkan* semaphore = (GPUSemaphore_Vulkan*)desc->signal_semaphore;
    GPUFence_Vulkan* F             = (GPUFence_Vulkan*)desc->fence;
    GPUDevice_Vulkan* D            = (GPUDevice_Vulkan*)swapchain->pDevice;

    VkResult result;
    uint32_t index = 0;

    VkSemaphore vks = semaphore ? semaphore->pVkSemaphore : VK_NULL_HANDLE;
    VkFence vkf     = F ? F->pVkFence : VK_NULL_HANDLE;
    result          = vkAcquireNextImageKHR(D->pDevice, S->pVkSwapchain, 0XFFFFFFFFFFFFFFFF, vks, vkf, &index);
    // If swapchain is out of date, let caller know by setting image index to -1
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        index = -1;
        if (F)
        {
            F->submitted = false;
            D->mVkDeviceTable.vkResetFences(D->pDevice, 1, &F->pVkFence);
        }
        if (semaphore) semaphore->signaled = false;
    }
    else if (result == VK_SUCCESS)
    {
        if (F) F->submitted = true;
        if (semaphore) semaphore->signaled = true;
    }
    return index;
}

GPUTextureViewID GPUCreateTextureView_Vulkan(GPUDeviceID pDevice, const GPUTextureViewDescriptor* pDesc)
{
    GPUDevice_Vulkan* pVkDevice           = (GPUDevice_Vulkan*)pDevice;
    GPUTextureView_Vulkan* pVkTextureView = (GPUTextureView_Vulkan*)_aligned_malloc(sizeof(GPUTextureView_Vulkan), alignof(GPUTextureView_Vulkan));

    GPUTexture_Vulkan* pVkTexture = (GPUTexture_Vulkan*)pDesc->pTexture;

    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkImageType imageType    = pDesc->pTexture->isCube ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            viewType = pDesc->arrayLayerCount > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        case VK_IMAGE_TYPE_2D:
            viewType = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case VK_IMAGE_TYPE_3D: {
            if (pDesc->arrayLayerCount > 1)
            {
                assert(0);
            }
            viewType = VK_IMAGE_VIEW_TYPE_3D;
        }
        break;
        default:
            assert(0);
            break;
    }
    assert(viewType != VK_IMAGE_VIEW_TYPE_MAX_ENUM);

    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_NONE;
    if (pDesc->aspectMask & EGPUTextureViewAspect::GPU_TVA_COLOR)
    {
        aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (pDesc->aspectMask & EGPUTextureViewAspect::GPU_TVA_DEPTH)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (pDesc->aspectMask & EGPUTextureViewAspect::GPU_TVA_STENCIL)
    {
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageViewCreateInfo createInfo{};
    createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image                           = pVkTexture->pVkImage;
    createInfo.viewType                        = viewType;
    createInfo.format                          = GPUFormatToVulkanFormat(pDesc->format);
    createInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
    createInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
    createInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
    createInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
    createInfo.subresourceRange.aspectMask     = aspectMask;
    createInfo.subresourceRange.baseMipLevel   = pDesc->baseMipLevel;
    createInfo.subresourceRange.levelCount     = pDesc->mipLevelCount;
    createInfo.subresourceRange.baseArrayLayer = pDesc->baseArrayLayer;
    createInfo.subresourceRange.layerCount     = pDesc->arrayLayerCount;

    pVkTextureView->pVkSRVDescriptor = VK_NULL_HANDLE;
    if (pDesc->usage & EGPUTexutreViewUsage::GPU_TVU_SRV)
    {
        if (pVkDevice->mVkDeviceTable.vkCreateImageView(pVkDevice->pDevice,
                                                        &createInfo,
                                                        GLOBAL_VkAllocationCallbacks,
                                                        &pVkTextureView->pVkSRVDescriptor) != VK_SUCCESS)
        {
            assert(0);
        }
    }
    pVkTextureView->pVkUAVDescriptor = VK_NULL_HANDLE;
    if (pDesc->usage & EGPUTexutreViewUsage::GPU_TVU_UAV)
    {
        VkImageViewCreateInfo tmp = createInfo;
        // #NOTE : We dont support imageCube, imageCubeArray for consistency with other APIs
        // All cubemaps will be used as image2DArray for Image Load / Store ops
        if (tmp.viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY || tmp.viewType == VK_IMAGE_VIEW_TYPE_CUBE)
        {
            tmp.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }
        tmp.subresourceRange.baseMipLevel = pDesc->baseMipLevel;
        if (pVkDevice->mVkDeviceTable.vkCreateImageView(pVkDevice->pDevice,
                                                        &tmp,
                                                        GLOBAL_VkAllocationCallbacks,
                                                        &pVkTextureView->pVkUAVDescriptor) != VK_SUCCESS)
        {
            assert(0);
        }
    }
    pVkTextureView->pVkRTVDSVDescriptor = VK_NULL_HANDLE;
    if (pDesc->usage & EGPUTexutreViewUsage::GPU_TVU_RTV_DSV)
    {
        if (pVkDevice->mVkDeviceTable.vkCreateImageView(pVkDevice->pDevice,
                                                        &createInfo,
                                                        GLOBAL_VkAllocationCallbacks,
                                                        &pVkTextureView->pVkRTVDSVDescriptor) != VK_SUCCESS)
        {
            assert(0);
        }
    }

    return &pVkTextureView->super;
}

void GPUFreeTextureView_Vulkan(GPUTextureViewID pTextureView)
{
    GPUDevice_Vulkan* pVkDevice  = (GPUDevice_Vulkan*)pTextureView->pDevice;
    GPUTextureView_Vulkan* pView = (GPUTextureView_Vulkan*)pTextureView;
    if (pView->pVkRTVDSVDescriptor != VK_NULL_HANDLE)
    {
        pVkDevice->mVkDeviceTable.vkDestroyImageView(pVkDevice->pDevice, pView->pVkRTVDSVDescriptor, GLOBAL_VkAllocationCallbacks);
    }
    if (pView->pVkSRVDescriptor != VK_NULL_HANDLE)
    {
        pVkDevice->mVkDeviceTable.vkDestroyImageView(pVkDevice->pDevice, pView->pVkSRVDescriptor, GLOBAL_VkAllocationCallbacks);
    }
    if (pView->pVkUAVDescriptor != VK_NULL_HANDLE)
    {
        pVkDevice->mVkDeviceTable.vkDestroyImageView(pVkDevice->pDevice, pView->pVkUAVDescriptor, GLOBAL_VkAllocationCallbacks);
    }

    _aligned_free(pView);
}

GPUShaderLibraryID GPUCreateShaderLibrary_Vulkan(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;

    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = pDesc->codeSize;
    info.pCode    = pDesc->code;

    GPUShaderLibrary_Vulkan* pShader = (GPUShaderLibrary_Vulkan*)calloc(1, sizeof(GPUShaderLibrary_Vulkan));
    if (!pDesc->reflectionOnly)
    {
        if (pVkDevice->mVkDeviceTable.vkCreateShaderModule(pVkDevice->pDevice, &info, GLOBAL_VkAllocationCallbacks, &pShader->pShader) != VK_SUCCESS)
        {
            assert(0);
        }
    }
    VulkanUtil_InitializeShaderReflection(pDevice, pShader, pDesc);
    return &pShader->super;
}

void GPUFreeShaderLibrary_Vulkan(GPUShaderLibraryID pShader)
{
    GPUShaderLibrary_Vulkan* pVkShader = (GPUShaderLibrary_Vulkan*)pShader;
    GPUDevice_Vulkan* pVkDevice        = (GPUDevice_Vulkan*)pShader->pDevice;
    VulkanUtil_FreeShaderReflection(pVkShader);
    pVkDevice->mVkDeviceTable.vkDestroyShaderModule(pVkDevice->pDevice, pVkShader->pShader, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(pVkShader);
}

static void FindOrCreateRenderPass(const GPUDevice_Vulkan* pDevice, const VulkanRenderPassDescriptor* pDesc, VkRenderPass* pVkPass)
{
    VkRenderPass found = VulkanUtil_RenderPassTableTryFind(pDevice->pPassTable, pDesc);
    if (found != VK_NULL_HANDLE)
    {
        *pVkPass = found;
        return;
    }
    uint32_t attachmentCount                                     = pDesc->attachmentCount;
    uint32_t depthCount                                          = (pDesc->depthFormat == GPU_FORMAT_UNDEFINED) ? 0 : 1;
    VkAttachmentDescription attachments[GPU_MAX_MRT_COUNT + 1]   = { 0 };
    VkAttachmentReference colorAttachmentRefs[GPU_MAX_MRT_COUNT] = { 0 };
    VkAttachmentReference depthStencilAttachmentRef[1]           = { 0 };

    uint32_t idx = 0;
    for (uint32_t i = 0; i < attachmentCount; i++)
    {
        attachments[idx].format        = GPUFormatToVulkanFormat(pDesc->pColorFormat[i]);
        attachments[idx].samples       = VulkanUtil_SampleCountToVk(pDesc->sampleCount);
        attachments[idx].loadOp        = gVkAttachmentLoadOpTranslator[pDesc->pColorLoadOps[i]];
        attachments[idx].storeOp       = gVkAttachmentStoreOpTranslator[pDesc->pColorStoreOps[i]];
        attachments[idx].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments[idx].finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // color references
        colorAttachmentRefs[idx].attachment = idx;
        colorAttachmentRefs[idx].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        idx++;
    }

    if (depthCount > 0)
    {
        attachments[idx].format         = GPUFormatToVulkanFormat(pDesc->depthFormat);
        attachments[idx].samples        = VulkanUtil_SampleCountToVk(pDesc->sampleCount);
        attachments[idx].loadOp         = gVkAttachmentLoadOpTranslator[pDesc->depthLoadOp];
        attachments[idx].storeOp        = gVkAttachmentStoreOpTranslator[pDesc->depthStoreOp];
        attachments[idx].stencilLoadOp  = gVkAttachmentLoadOpTranslator[pDesc->stencilLoadOp];
        attachments[idx].stencilStoreOp = gVkAttachmentStoreOpTranslator[pDesc->stencilStoreOp];
        attachments[idx].initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[idx].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        // depth reference
        depthStencilAttachmentRef[0].attachment = idx;
        depthStencilAttachmentRef[0].layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        idx++;
    }

    uint32_t _attachmentCount = attachmentCount;
    _attachmentCount += depthCount;
    VkSubpassDescription subPass{};
    subPass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount    = _attachmentCount;
    subPass.pColorAttachments       = colorAttachmentRefs;
    subPass.pDepthStencilAttachment = (depthCount > 0) ? depthStencilAttachmentRef : VK_NULL_HANDLE;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = _attachmentCount; // color attachment + depth attachment
    createInfo.pAttachments    = attachments;
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subPass;
    createInfo.dependencyCount = 0;
    createInfo.pDependencies   = VK_NULL_HANDLE;

    VkResult rs = pDevice->mVkDeviceTable.vkCreateRenderPass(pDevice->pDevice, &createInfo, GLOBAL_VkAllocationCallbacks, pVkPass);
    assert(rs == VK_SUCCESS);
    VulkanUtil_RenderPassTableAdd(pDevice->pPassTable, pDesc, *pVkPass);
}

static void FindOrCreateFrameBuffer(const GPUDevice_Vulkan* D, const struct VulkanFramebufferDesriptor* pDesc, VkFramebuffer* ppFramebuffer)
{
    VkFramebuffer found = VulkanUtil_FrameBufferTableTryFind(D->pPassTable, pDesc);
    if (found != VK_NULL_HANDLE)
    {
        *ppFramebuffer = found;
        return;
    }
    assert(VK_NULL_HANDLE != D->pDevice);
    VkFramebufferCreateInfo add_info = {
        .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext           = NULL,
        .flags           = 0,
        .renderPass      = pDesc->pRenderPass,
        .attachmentCount = pDesc->attachmentCount,
        .pAttachments    = pDesc->pImageViews,
        .width           = pDesc->width,
        .height          = pDesc->height,
        .layers          = pDesc->layers
    };
    assert(vkCreateFramebuffer(D->pDevice, &add_info, GLOBAL_VkAllocationCallbacks, ppFramebuffer) == VK_SUCCESS);
    VulkanUtil_FrameBufferTableAdd(D->pPassTable, pDesc, *ppFramebuffer);
}

GPURenderPipelineID GPUCreateRenderPipeline_Vulkan(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;
    GPURootSignature_Vulkan* pVkRS = (GPURootSignature_Vulkan*)pDesc->pRootSignature;

    // Vertex input state
    uint32_t inputBindingCount = 0;
    uint32_t inputAttribCount  = 0;
    uint32_t attrCount         = pDesc->pVertexLayout->attributeCount > GPU_MAX_VERTEX_ATTRIBS ? GPU_MAX_VERTEX_ATTRIBS : pDesc->pVertexLayout->attributeCount;
    uint32_t attribVal         = 0xffffffff;
    for (uint32_t i = 0; i < attrCount; i++)
    {
        const GPUVertexAttribute* pAttrib = &pDesc->pVertexLayout->attributes[i];
        uint32_t arraySize                = pAttrib->arraySize ? pAttrib->arraySize : 1;
        if (attribVal != pAttrib->binding)
        {
            attribVal = pAttrib->binding;
            inputBindingCount++;
        }
        for (uint32_t j = 0; j < arraySize; j++)
        {
            inputAttribCount += 1;
        }
    }
    uint64_t dsize = sizeof(GPURenderPipeline_Vulkan);
    uint64_t inputElemOffset = dsize;
    dsize += (sizeof(VkVertexInputBindingDescription) * inputBindingCount);
    uint64_t inputAttribsOffset = dsize;
    dsize += (sizeof(VkVertexInputAttributeDescription) * inputAttribCount);
    uint8_t* ptr                                   = (uint8_t*)malloc(dsize);
    GPURenderPipeline_Vulkan* pRp                  = (GPURenderPipeline_Vulkan*)ptr;
    VkVertexInputBindingDescription* pBindingDesc  = (VkVertexInputBindingDescription*)(ptr + inputElemOffset);
    VkVertexInputAttributeDescription* pAttribDesc = (VkVertexInputAttributeDescription*)(ptr + inputAttribsOffset);

    uint32_t slot = 0;
    for (uint32_t i = 0; i < attrCount; i++)
    {
        const GPUVertexAttribute* pAttrib         = &pDesc->pVertexLayout->attributes[i];
        uint32_t arraySize                        = pAttrib->arraySize ? pAttrib->arraySize : 1;
        VkVertexInputBindingDescription* bindDesc = &pBindingDesc[i];
        bindDesc->binding                         = pAttrib->binding;
        if (pAttrib->rate == EGPUVertexInputRate::GPU_INPUT_RATE_VERTEX)
        {
            bindDesc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }
        else
        {
            bindDesc->inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        }
        bindDesc->stride = pAttrib->stride;

        for (uint32_t j = 0; j < arraySize; j++)
        {
            pAttribDesc[slot].location = slot;
            pAttribDesc[slot].binding  = pAttrib->binding;
            pAttribDesc[slot].format   = GPUFormatToVulkanFormat(pAttrib->format);
            pAttribDesc[slot].offset   = pAttrib->offset + (j * VulkanUtil_BitSizeOfBlock(pAttrib->format) / 8);
            slot++;
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = inputBindingCount;
    vertexInputInfo.pVertexBindingDescriptions      = pBindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = inputAttribCount;
    vertexInputInfo.pVertexAttributeDescriptions    = pAttribDesc;

    // shader stage
    uint32_t shaderStagCount = 2;
    DECLEAR_ZERO_VAL(VkPipelineShaderStageCreateInfo, shaderStage, shaderStagCount);
    shaderStage[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStage[0].module = ((GPUShaderLibrary_Vulkan*)(pDesc->pVertexShader->pLibrary))->pShader;
    shaderStage[0].pName  = (const char*)pDesc->pVertexShader->entry;
    shaderStage[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStage[1].module = ((GPUShaderLibrary_Vulkan*)(pDesc->pFragmentShader->pLibrary))->pShader;
    shaderStage[1].pName  = (const char*)pDesc->pFragmentShader->entry;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewPort{};
    viewPort.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewPort.viewportCount = 1;
    viewPort.pViewports    = VK_NULL_HANDLE;
    viewPort.scissorCount  = 1;
    viewPort.pScissors     = VK_NULL_HANDLE;

    VkDynamicState dyn_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
//#if VK_KHR_fragment_shading_rate
//        VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR
//#endif
    };
    VkPipelineDynamicStateCreateInfo dyInfo {};
    dyInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyInfo.dynamicStateCount = sizeof(dyn_states) / sizeof(VkDynamicState);
    dyInfo.pDynamicStates    = dyn_states;

    // Multi-sampling
    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples  = VulkanUtil_SampleCountToVk(pDesc->samplerCount);
    ms.sampleShadingEnable   = VK_FALSE;
    ms.minSampleShading      = 0.f;
    ms.pSampleMask           = 0;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable      = VK_FALSE;

    //ia
    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology               = VulkanUtil_PrimitiveTopologyToVk(pDesc->primitiveTopology);
    ia.primitiveRestartEnable = VK_FALSE;

    // Depth stencil state
    VkPipelineDepthStencilStateCreateInfo dss{};
    dss.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    dss.depthTestEnable       = pDesc->pDepthState->depthTest ? VK_TRUE : VK_FALSE;
    dss.depthWriteEnable      = pDesc->pDepthState->depthWrite ? VK_TRUE : VK_FALSE;
    dss.depthCompareOp        = VulkanUtil_CompareOpToVk(pDesc->pDepthState->depthFunc);
    dss.depthBoundsTestEnable = VK_FALSE;
    dss.stencilTestEnable     = pDesc->pDepthState->stencilTest ? VK_TRUE : VK_FALSE;
    dss.front.failOp          = VulkanUtil_StencilOpToVk(pDesc->pDepthState->stencilFrontFail);
    dss.front.passOp          = VulkanUtil_StencilOpToVk(pDesc->pDepthState->stencilFrontPass);
    dss.front.depthFailOp     = VulkanUtil_StencilOpToVk(pDesc->pDepthState->depthFrontFail);
    dss.front.compareOp       = VulkanUtil_CompareOpToVk(pDesc->pDepthState->stencilFrontFunc);
    dss.front.compareMask     = pDesc->pDepthState->stencilReadMask;
    dss.front.writeMask       = pDesc->pDepthState->stencilWriteMask;
    dss.front.reference       = 0;
    dss.back.failOp           = VulkanUtil_StencilOpToVk(pDesc->pDepthState->stencilBackFail);
    dss.back.passOp           = VulkanUtil_StencilOpToVk(pDesc->pDepthState->stencilBackPass);
    dss.back.depthFailOp      = VulkanUtil_StencilOpToVk(pDesc->pDepthState->depthBackFail);
    dss.back.compareOp        = VulkanUtil_CompareOpToVk(pDesc->pDepthState->stencilBackFunc);
    dss.back.compareMask      = pDesc->pDepthState->stencilReadMask;
    dss.back.writeMask        = pDesc->pDepthState->stencilWriteMask;
    dss.back.reference        = 0;
    dss.minDepthBounds        = 0.f;
    dss.maxDepthBounds        = 1.f;

    // Rasterizer state
    uint32_t depthBias = pDesc->pRasterizerState ? pDesc->pRasterizerState->depthBias : 0;
    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.depthClampEnable        = pDesc->pRasterizerState ? (pDesc->pRasterizerState->enableDepthClamp ? VK_TRUE : VK_FALSE) : VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.polygonMode             = pDesc->pRasterizerState ? gVkFillModeTranslator[pDesc->pRasterizerState->fillMode] : VK_POLYGON_MODE_FILL;
    rs.cullMode                = pDesc->pRasterizerState ? gVkCullModeTranslator[pDesc->pRasterizerState->cullMode] : VK_CULL_MODE_BACK_BIT;
    rs.frontFace               = pDesc->pRasterizerState ? gVkFrontFaceTranslator[pDesc->pRasterizerState->frontFace] : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthBiasEnable         = (depthBias != 0) ? VK_TRUE : VK_FALSE;
    rs.depthBiasConstantFactor = pDesc->pRasterizerState ? pDesc->pRasterizerState->depthBias : 0.f;
    rs.depthBiasClamp          = 0.f;
    rs.depthBiasSlopeFactor    = pDesc->pRasterizerState ? pDesc->pRasterizerState->slopeScaledDepthBias : 0.f;
    rs.lineWidth               = 1.f;

     // Color blending state
    VkPipelineColorBlendAttachmentState colorBlendattachments[GPU_MAX_MRT_COUNT] = {};
    uint32_t blendDescIndex = 0;
    for (int i = 0; i < GPU_MAX_MRT_COUNT; ++i)
    {
        VkBool32 blendEnable =
        (gVkBlendConstantTranslator[pDesc->pBlendState->srcFactors[blendDescIndex]] != VK_BLEND_FACTOR_ONE ||
         gVkBlendConstantTranslator[pDesc->pBlendState->dstFactors[blendDescIndex]] != VK_BLEND_FACTOR_ZERO ||
         gVkBlendConstantTranslator[pDesc->pBlendState->srcAlphaFactors[blendDescIndex]] != VK_BLEND_FACTOR_ONE ||
         gVkBlendConstantTranslator[pDesc->pBlendState->dstAlphaFactors[blendDescIndex]] != VK_BLEND_FACTOR_ZERO);

        colorBlendattachments[i].blendEnable         = blendEnable;
        colorBlendattachments[i].srcColorBlendFactor = gVkBlendConstantTranslator[pDesc->pBlendState->srcFactors[blendDescIndex]];
        colorBlendattachments[i].dstColorBlendFactor = gVkBlendConstantTranslator[pDesc->pBlendState->dstFactors[blendDescIndex]];
        colorBlendattachments[i].colorBlendOp        = gVkBlendOpTranslator[pDesc->pBlendState->blendModes[blendDescIndex]];
        colorBlendattachments[i].srcAlphaBlendFactor = gVkBlendConstantTranslator[pDesc->pBlendState->srcAlphaFactors[blendDescIndex]];
        colorBlendattachments[i].dstAlphaBlendFactor = gVkBlendConstantTranslator[pDesc->pBlendState->dstAlphaFactors[blendDescIndex]];
        colorBlendattachments[i].alphaBlendOp        = gVkBlendOpTranslator[pDesc->pBlendState->blendAlphaModes[blendDescIndex]];
        colorBlendattachments[i].colorWriteMask      = pDesc->pBlendState->masks[blendDescIndex];

        if (pDesc->pBlendState->independentBlend)
            ++blendDescIndex;
    }
    VkPipelineColorBlendStateCreateInfo cbs = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext             = NULL,
        .flags             = 0,
        .logicOpEnable     = VK_FALSE,
        .logicOp           = VK_LOGIC_OP_COPY,
        .attachmentCount   = pDesc->renderTargetCount,
        .pAttachments      = colorBlendattachments
    };
    cbs.blendConstants[0] = 0.0f,
    cbs.blendConstants[1] = 0.0f,
    cbs.blendConstants[2] = 0.0f,
    cbs.blendConstants[3] = 0.0f;

    assert(pDesc->renderTargetCount > 0);
    VkRenderPass pRenderPass = VK_NULL_HANDLE;
    VulkanRenderPassDescriptor renderPassDesc{};
    renderPassDesc.attachmentCount = pDesc->renderTargetCount;
    renderPassDesc.sampleCount     = pDesc->samplerCount;
    renderPassDesc.depthFormat     = pDesc->depthStencilFormat;
    for (uint32_t i = 0; i < pDesc->renderTargetCount; i++)
    {
        renderPassDesc.pColorFormat[i] = pDesc->pColorFormats[i];
    }
    FindOrCreateRenderPass(pVkDevice, &renderPassDesc, &pRenderPass);
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = shaderStagCount;
    pipelineCreateInfo.pStages    = shaderStage;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &ia;
    pipelineCreateInfo.pTessellationState  = VK_NULL_HANDLE;
    pipelineCreateInfo.pViewportState      = &viewPort;
    pipelineCreateInfo.pRasterizationState = &rs;
    pipelineCreateInfo.pMultisampleState   = &ms;
    pipelineCreateInfo.pDepthStencilState  = &dss;
    pipelineCreateInfo.pColorBlendState    = &cbs;
    pipelineCreateInfo.pDynamicState       = &dyInfo;
    pipelineCreateInfo.layout              = pVkRS->pPipelineLayout;
    pipelineCreateInfo.renderPass          = pRenderPass;
    pipelineCreateInfo.subpass             = 0;

    VkResult result = pVkDevice->mVkDeviceTable.vkCreateGraphicsPipelines(pVkDevice->pDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, GLOBAL_VkAllocationCallbacks, &pRp->pPipeline);
    assert(result == VK_SUCCESS);

    return &pRp->super;
}

void GPUFreeRenderPipeline_Vulkan(GPURenderPipelineID pPipeline)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pPipeline->pDevice;
    GPURenderPipeline_Vulkan* pVkRpr = (GPURenderPipeline_Vulkan*)pPipeline;

    pVkDevice->mVkDeviceTable.vkDestroyPipeline(pVkDevice->pDevice, pVkRpr->pPipeline, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(pVkRpr);
}

GPURootSignatureID GPUCreateRootSignature_Vulkan(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc)
{
    const GPUDevice_Vulkan* D   = (GPUDevice_Vulkan*)device;
    GPURootSignature_Vulkan* RS = (GPURootSignature_Vulkan*)malloc(sizeof(GPURootSignature_Vulkan));
    memset(RS, 0, sizeof(GPURootSignature_Vulkan));
    CGPUUtil_InitRSParamTables((GPURootSignature*)RS, desc);
    // [RS POOL] ALLOCATION
    if (desc->pool)
    {
        /*GPURootSignature_Vulkan* poolSig =
        (GPURootSignature_Vulkan*)GPUUtil_TryAllocateSignature(desc->pool, &RS->super, desc);
        if (poolSig != VK_NULL_HANDLE)
        {
            RS->pPipelineLayout  = poolSig->pPipelineLayout;
            RS->pSetLayouts      = poolSig->pSetLayouts;
            RS->setLayoutsCount  = poolSig->setLayoutsCount;
            RS->pPushConstantRanges = poolSig->pPushConstantRanges;
            RS->super.pool       = desc->pool;
            RS->super.pool_sig   = &poolSig->super;
            return &RS->super;
        }*/
    }
    // [RS POOL] END ALLOCATION
    // set index mask. set(0, 1, 2, 3) -> 0000...1111
    uint32_t set_index_mask = 0;
    // tables
    for (uint32_t i = 0; i < RS->super.table_count; i++)
    {
        set_index_mask |= (1 << RS->super.tables[i].set_index);
    }
    // static samplers
    for (uint32_t i = 0; i < RS->super.static_sampler_count; i++)
    {
        set_index_mask |= (1 << RS->super.static_samplers[i].set);
    }
    // parse
    //const uint32_t set_count = get_set_count(set_index_mask);
    uint32_t set_count = 0;
    uint32_t m         = set_index_mask;
    while (m != 0)
    {
        if (m & 1)
        {
            set_count++;
        }
        m >>= 1;
    }
    RS->pSetLayouts          = (SetLayout_Vulkan*)malloc(set_count * sizeof(SetLayout_Vulkan));
    RS->setLayoutsCount      = set_count;
    uint32_t set_index       = 0;
    while (set_index_mask != 0)
    {
        if (set_index_mask & 1)
        {
            GPUParameterTable* param_table = NULL;
            for (uint32_t i = 0; i < RS->super.table_count; i++)
            {
                if (RS->super.tables[i].set_index == set_index)
                {
                    param_table = &RS->super.tables[i];
                    break;
                }
            }
            uint32_t bindings_count                  = param_table ? param_table->resources_count + desc->static_sampler_count : 0 + desc->static_sampler_count;
            VkDescriptorSetLayoutBinding* vkbindings = (VkDescriptorSetLayoutBinding*)malloc(bindings_count * sizeof(VkDescriptorSetLayoutBinding));
            uint32_t i_binding = 0;
            // bindings
            if (param_table)
            {
                for (i_binding = 0; i_binding < param_table->resources_count; i_binding++)
                {
                    vkbindings[i_binding].binding         = param_table->resources[i_binding].binding;
                    vkbindings[i_binding].stageFlags      = VulkanUtil_TranslateShaderUsages(param_table->resources[i_binding].stages);
                    vkbindings[i_binding].descriptorType  = VulkanUtil_TranslateResourceType(param_table->resources[i_binding].type);
                    vkbindings[i_binding].descriptorCount = param_table->resources[i_binding].size;
                }
            }
            // static samplers
            /*for (uint32_t i_ss = 0; i_ss < desc->static_sampler_count; i_ss++)
            {
                if (RS->super.static_samplers[i_ss].set == set_index)
                {
                    GPUSampler_Vulkan* immutableSampler     = (GPUSampler_Vulkan*)desc->static_samplers[i_ss];
                    vkbindings[i_binding].pImmutableSamplers = &immutableSampler->pVkSampler;
                    vkbindings[i_binding].binding            = RS->super.static_samplers[i_ss].binding;
                    vkbindings[i_binding].stageFlags         = VkUtil_TranslateShaderUsages(RS->super.static_samplers[i_ss].stages);
                    vkbindings[i_binding].descriptorType     = VkUtil_TranslateResourceType(RS->super.static_samplers[i_ss].type);
                    vkbindings[i_binding].descriptorCount    = RS->super.static_samplers[i_ss].size;
                    i_binding++;
                }
            }*/
            VkDescriptorSetLayoutCreateInfo set_info = {
                .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext        = NULL,
                .flags        = 0,
                .bindingCount = i_binding,
                .pBindings    = vkbindings
            };
            assert(D->mVkDeviceTable.vkCreateDescriptorSetLayout(D->pDevice,
                                                                 &set_info,
                                                                 GLOBAL_VkAllocationCallbacks,
                                                                 &RS->pSetLayouts[set_index].pLayout) == VK_SUCCESS);
            //vkAllocateDescriptorSets
            VkDescriptorSetAllocateInfo setsAllocInfo{};
            setsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            setsAllocInfo.descriptorPool = D->pDescriptorPool->pVkDescPool;
            setsAllocInfo.descriptorSetCount = 1;
            setsAllocInfo.pSetLayouts        = &RS->pSetLayouts[set_index].pLayout;
            assert(D->mVkDeviceTable.vkAllocateDescriptorSets(D->pDevice, &setsAllocInfo, &RS->pSetLayouts[set_index].pEmptyDescSet) == VK_SUCCESS);

            if (bindings_count) free(vkbindings);
        }
        set_index++;
        set_index_mask >>= 1;
    }
    // Push constants
    // Collect push constants count
    /* if (RS->super.push_constant_count > 0)
    {
        RS->pPushConstRanges = (VkPushConstantRange*)cgpu_calloc(RS->super.push_constant_count, sizeof(VkPushConstantRange));
        // Create Vk Objects
        for (uint32_t i_const = 0; i_const < RS->super.push_constant_count; i_const++)
        {
            RS->pPushConstRanges[i_const].stageFlags =
            VkUtil_TranslateShaderUsages(RS->super.push_constants[i_const].stages);
            RS->pPushConstRanges[i_const].size   = RS->super.push_constants[i_const].size;
            RS->pPushConstRanges[i_const].offset = RS->super.push_constants[i_const].offset;
        }
    }*/
    // Record Descriptor Sets
    RS->pVkSetLayouts = (VkDescriptorSetLayout*)malloc(set_count * sizeof(VkDescriptorSetLayout));
    for (uint32_t i_set = 0; i_set < set_count; i_set++)
    {
        SetLayout_Vulkan* set_to_record = (SetLayout_Vulkan*)&RS->pSetLayouts[i_set];
        RS->pVkSetLayouts[i_set]        = set_to_record->pLayout;
    }
    // Create Pipeline Layout
    VkPipelineLayoutCreateInfo pipeline_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = NULL,
        .flags                  = 0,
        .setLayoutCount         = set_count,
        .pSetLayouts            = RS->pVkSetLayouts,
        .pushConstantRangeCount = RS->super.push_constant_count,
        //.pPushConstantRanges    = RS->pPushConstRanges
    };
    assert(D->mVkDeviceTable.vkCreatePipelineLayout(D->pDevice, &pipeline_info, GLOBAL_VkAllocationCallbacks, &RS->pPipelineLayout) == VK_SUCCESS);
    // Create Update Templates
    for (uint32_t i_table = 0; i_table < RS->super.table_count; i_table++)
    {
        GPUParameterTable* param_table                    = &RS->super.tables[i_table];
        SetLayout_Vulkan* set_to_record                   = &RS->pSetLayouts[param_table->set_index];
        uint32_t update_entry_count                       = param_table->resources_count;
        VkDescriptorUpdateTemplateEntry* template_entries = (VkDescriptorUpdateTemplateEntry*)calloc(
        param_table->resources_count, sizeof(VkDescriptorUpdateTemplateEntry));
        for (uint32_t i_iter = 0; i_iter < param_table->resources_count; i_iter++)
        {
            uint32_t i_binding                          = param_table->resources[i_iter].binding;
            VkDescriptorUpdateTemplateEntry* this_entry = template_entries + i_iter;
            this_entry->descriptorCount                 = param_table->resources[i_iter].size;
            this_entry->descriptorType                  = VulkanUtil_TranslateResourceType(param_table->resources[i_iter].type);
            this_entry->dstBinding                      = i_binding;
            this_entry->dstArrayElement                 = 0;
            this_entry->stride                          = sizeof(VkDescriptorUpdateData);
            this_entry->offset                          = this_entry->dstBinding * this_entry->stride;
        }
        if (update_entry_count > 0)
        {
            VkDescriptorUpdateTemplateCreateInfo template_info = {
                .sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
                .pNext                      = NULL,
                .descriptorUpdateEntryCount = update_entry_count,
                .pDescriptorUpdateEntries   = template_entries,
                .templateType               = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET_KHR,
                .descriptorSetLayout        = set_to_record->pLayout,
                .pipelineBindPoint          = gPipelineBindPoint[RS->super.pipeline_type],
                .pipelineLayout             = RS->pPipelineLayout,
                .set                        = param_table->set_index
            };
            set_to_record->updateEntriesCount = update_entry_count;
            assert(D->mVkDeviceTable.vkCreateDescriptorUpdateTemplate(
                D->pDevice,
                &template_info, GLOBAL_VkAllocationCallbacks, &set_to_record->pUpdateTemplate) == VK_SUCCESS);
        }
        free(template_entries);
    }
    // [RS POOL] INSERTION
    if (desc->pool)
    {
        /*const bool result = CGPUUtil_AddSignature(desc->pool, &RS->super, desc);
        cgpu_assert(result && "Root signature pool insertion failed!");*/
    }
    // [RS POOL] END INSERTION
    return &RS->super;
}

void GPUFreeRootSignature_Vulkan(GPURootSignatureID RS)
{
    GPUDevice_Vulkan* D           = (GPUDevice_Vulkan*)RS->device;
    GPURootSignature_Vulkan* vkRS = (GPURootSignature_Vulkan*)RS;
    GPUUtil_FreeRSParamTables((GPURootSignature*)RS);

    // Free Vk Objects
    for (uint32_t i_set = 0; i_set < vkRS->setLayoutsCount; i_set++)
    {
        SetLayout_Vulkan* set_to_free = &vkRS->pSetLayouts[i_set];
        if (set_to_free->pLayout != VK_NULL_HANDLE)
            D->mVkDeviceTable.vkDestroyDescriptorSetLayout(D->pDevice, set_to_free->pLayout, GLOBAL_VkAllocationCallbacks);
        if (set_to_free->pUpdateTemplate != VK_NULL_HANDLE)
            D->mVkDeviceTable.vkDestroyDescriptorUpdateTemplate(D->pDevice, set_to_free->pUpdateTemplate, GLOBAL_VkAllocationCallbacks);
    }
    GPU_SAFE_FREE(vkRS->pVkSetLayouts);
    GPU_SAFE_FREE(vkRS->pSetLayouts);
    GPU_SAFE_FREE(vkRS->pPushConstantRanges);
    D->mVkDeviceTable.vkDestroyPipelineLayout(D->pDevice, vkRS->pPipelineLayout, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(vkRS);

}

bool CGPUUtil_ShaderResourceIsRootConst(GPUShaderResource* resource, const struct GPURootSignatureDescriptor* desc)
{
    if (resource->type == GPU_RESOURCE_TYPE_PUSH_CONSTANT) return true;
    for (uint32_t i = 0; i < desc->push_constant_count; i++)
    {
        if (strcmp((const char*)resource->name, (const char*)desc->push_constant_names[i]) == 0)
            return true;
    }
    return false;
}

bool CGPUUtil_ShaderResourceIsStaticSampler(GPUShaderResource* resource, const struct GPURootSignatureDescriptor* desc)
{
    for (uint32_t i = 0; i < desc->static_sampler_count; i++)
    {
        if (strcmp((const char*)resource->name, (const char*)desc->static_sampler_names[i]) == 0)
        {
            return resource->type == GPU_RESOURCE_TYPE_SAMPLER;
        }
    }
    return false;
}
inline static char8_t* duplicate_string(const char8_t* src_string)
{
    if (src_string != NULL)
    {
        const size_t source_len = strlen((const char*)src_string);
        char8_t* result         = (char8_t*)malloc(sizeof(char8_t) * (1 + source_len));
#ifdef _WIN32
        strcpy_s((char*)result, source_len + 1, (const char*)src_string);
#else
        strcpy((char*)result, (const char*)src_string);
#endif
        return result;
    }
    return NULL;
}

#include <set>
// 这是一个非常复杂的过程，牵扯到大量的move和join操作。具体的逻辑如下：
// 1.收集所有ShaderStage中出现的所有ShaderResource，不管他们是否重复
//   这个阶段也会收集所有的RootConst，并且对它们进行合并（不同阶段出现的相同RootConst的Stage合并）
//   也会收集所有的StaticSamplers
// 2.合并ShaderResources到RootSignatureTable（下称为RST）中
//   拥有相同set、binding以及type的、出现在不同ShaderStage中的ShaderResource会被合并Stage
// 3.切分行
//   按照Set把合并好的Resource分割并放进roosting::tables中
void CGPUUtil_InitRSParamTables(GPURootSignature* RS, const struct GPURootSignatureDescriptor* desc)
{
    GPUShaderReflection* entry_reflections[32] = { 0 };
    // Pick shader reflection data
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        const GPUShaderEntryDescriptor* shader_desc = &desc->shaders[i];
        // Find shader reflection
        for (uint32_t j = 0; j < shader_desc->pLibrary->entrys_count; j++)
        {
            GPUShaderReflection* temp_entry_reflcetion = &shader_desc->pLibrary->entry_reflections[j];
            if (temp_entry_reflcetion->entry_name)
            {
                if (strcmp((const char*)shader_desc->entry, (const char*)temp_entry_reflcetion->entry_name) == 0)
                {
                    entry_reflections[i] = temp_entry_reflcetion;
                    break;
                }
            }
        }
        if (entry_reflections[i] == NULL)
        {
            entry_reflections[i] = &shader_desc->pLibrary->entry_reflections[0];
        }
    }
    // Collect all resources
    RS->pipeline_type = GPU_PIPELINE_TYPE_NONE;
    std::vector<GPUShaderResource> all_resources;
    std::vector<GPUShaderResource> all_push_constants;
    std::vector<GPUShaderResource> all_static_samplers;
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        GPUShaderReflection* reflection = entry_reflections[i];
        for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
        {
            GPUShaderResource& resource = reflection->shader_resources[j];
            if (CGPUUtil_ShaderResourceIsRootConst(&resource, desc))
            {
                bool coincided = false;
                for (auto&& root_const : all_push_constants)
                {
                    if (root_const.name_hash == resource.name_hash &&
                        root_const.set == resource.set &&
                        root_const.binding == resource.binding &&
                        root_const.size == resource.size)
                    {
                        root_const.stages |= resource.stages;
                        coincided = true;
                    }
                }
                if (!coincided)
                    all_push_constants.emplace_back(resource);
            }
            else if (CGPUUtil_ShaderResourceIsStaticSampler(&resource, desc))
            {
                bool coincided = false;
                for (auto&& static_sampler : all_static_samplers)
                {
                    if (static_sampler.name_hash == resource.name_hash &&
                        static_sampler.set == resource.set &&
                        static_sampler.binding == resource.binding)
                    {
                        static_sampler.stages |= resource.stages;
                        coincided = true;
                    }
                }
                if (!coincided)
                    all_static_samplers.emplace_back(resource);
            }
            else
            {
                all_resources.emplace_back(resource);
            }
        }
        // Pipeline Type
        if (reflection->stage & GPU_SHADER_STAGE_COMPUTE)
            RS->pipeline_type = GPU_PIPELINE_TYPE_COMPUTE;
        else if (reflection->stage & GPU_SHADER_STAGE_RAYTRACING)
            RS->pipeline_type = GPU_PIPELINE_TYPE_RAYTRACING;
        else
            RS->pipeline_type = GPU_PIPELINE_TYPE_GRAPHICS;
    }
    // Merge
    std::set<uint32_t> valid_sets;
    std::vector<GPUShaderResource> RST_resources;
    RST_resources.reserve(all_resources.size());
    for (auto&& shader_resource : all_resources)
    {
        bool coincided = false;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == shader_resource.set &&
                RST_resource.binding == shader_resource.binding &&
                RST_resource.type == shader_resource.type)
            {
                RST_resource.stages |= shader_resource.stages;
                coincided = true;
            }
        }
        if (!coincided)
        {
            valid_sets.insert(shader_resource.set);
            RST_resources.emplace_back(shader_resource);
        }
    }
    std::stable_sort(RST_resources.begin(), RST_resources.end(),
                       [](const GPUShaderResource& lhs, const GPUShaderResource& rhs) {
                           if (lhs.set != rhs.set)
                               return lhs.set < rhs.set;
                           else
                               return lhs.binding < rhs.binding;
                       });
    // Slice
    RS->table_count      = (uint32_t)valid_sets.size();
    RS->tables           = (GPUParameterTable*)malloc(RS->table_count * sizeof(GPUParameterTable));
    uint32_t table_index = 0;
    for (auto set_index : valid_sets)
    {
        GPUParameterTable& table = RS->tables[table_index];
        table.set_index           = set_index;
        table.resources_count     = 0;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == set_index)
                table.resources_count++;
        }
        table.resources = (GPUShaderResource*)malloc(
        table.resources_count * sizeof(GPUShaderResource));
        uint32_t slot_index = 0;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == set_index)
            {
                table.resources[slot_index] = RST_resource;
                slot_index++;
            }
        }
        table_index++;
    }
    // push constants
    RS->push_constant_count = (uint32_t)all_push_constants.size();
    RS->push_constants      = (GPUShaderResource*)malloc(
    RS->push_constant_count * sizeof(GPUShaderResource));
    for (uint32_t i = 0; i < all_push_constants.size(); i++)
    {
        RS->push_constants[i] = all_push_constants[i];
    }
    // static samplers
    std::stable_sort(all_static_samplers.begin(), all_static_samplers.end(),
                       [](const GPUShaderResource& lhs, const GPUShaderResource& rhs) {
                           if (lhs.set != rhs.set)
                               return lhs.set < rhs.set;
                           else
                               return lhs.binding < rhs.binding;
                       });
    RS->static_sampler_count = (uint32_t)all_static_samplers.size();
    RS->static_samplers      = (GPUShaderResource*)malloc(
    RS->static_sampler_count * sizeof(GPUShaderResource));
    for (uint32_t i = 0; i < all_static_samplers.size(); i++)
    {
        RS->static_samplers[i] = all_static_samplers[i];
    }
    // copy names
    for (uint32_t i = 0; i < RS->push_constant_count; i++)
    {
        GPUShaderResource* dst = RS->push_constants + i;
        dst->name               = duplicate_string(dst->name);
    }
    for (uint32_t i = 0; i < RS->static_sampler_count; i++)
    {
        GPUShaderResource* dst = RS->static_samplers + i;
        dst->name               = duplicate_string(dst->name);
    }
    for (uint32_t i = 0; i < RS->table_count; i++)
    {
        GPUParameterTable* set_to_record = &RS->tables[i];
        for (uint32_t j = 0; j < set_to_record->resources_count; j++)
        {
            GPUShaderResource* dst = &set_to_record->resources[j];
            dst->name               = duplicate_string(dst->name);
        }
    }
}

void GPUUtil_FreeRSParamTables(GPURootSignature* RS)
{
    if (RS->tables != NULL)
    {
        for (uint32_t i_set = 0; i_set < RS->table_count; i_set++)
        {
            GPUParameterTable* param_table = &RS->tables[i_set];
            if (param_table->resources != NULL)
            {
                for (uint32_t i_binding = 0; i_binding < param_table->resources_count; i_binding++)
                {
                    GPUShaderResource* binding_to_free = &param_table->resources[i_binding];
                    if (binding_to_free->name != NULL)
                    {
                        free((char8_t*)binding_to_free->name);
                    }
                }
                free(param_table->resources);
            }
        }
        free(RS->tables);
    }
    if (RS->push_constants != NULL)
    {
        for (uint32_t i = 0; i < RS->push_constant_count; i++)
        {
            GPUShaderResource* binding_to_free = RS->push_constants + i;
            if (binding_to_free->name != NULL)
            {
                free((char8_t*)binding_to_free->name);
            }
        }
        free(RS->push_constants);
    }
    if (RS->static_samplers != NULL)
    {
        for (uint32_t i = 0; i < RS->static_sampler_count; i++)
        {
            GPUShaderResource* binding_to_free = RS->static_samplers + i;
            if (binding_to_free->name != NULL)
            {
                free((char8_t*)binding_to_free->name);
            }
        }
        free(RS->static_samplers);
    }
}


GPUCommandPoolID GPUCreateCommandPool_Vulkan(GPUQueueID queue)
{
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)queue->pDevice;
    GPUCommandPool_Vulkan* pPool = (GPUCommandPool_Vulkan*)calloc(1, sizeof(GPUCommandPool_Vulkan));
    pPool->pPool                 = AllocateTransientCommandPool(D, queue);
    return &pPool->super;
}

VkCommandPool AllocateTransientCommandPool(struct GPUDevice_Vulkan* D, GPUQueueID queue)
{
    VkCommandPool p = VK_NULL_HANDLE;

    VkCommandPoolCreateInfo info{};
    info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.queueFamilyIndex = queue->queueIndex;
    assert(D->mVkDeviceTable.vkCreateCommandPool(D->pDevice, &info, GLOBAL_VkAllocationCallbacks, &p) == VK_SUCCESS);

    return p;
}

void GPUFreeCommandPool_Vulkan(GPUCommandPoolID pool)
{
    GPUDevice_Vulkan* D      = (GPUDevice_Vulkan*)pool->queue->pDevice;
    GPUCommandPool_Vulkan* P = (GPUCommandPool_Vulkan*)pool;
    D->mVkDeviceTable.vkDestroyCommandPool(D->pDevice, P->pPool, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(P);
}

void GPUResetCommandPool_Vulkan(GPUCommandPoolID pool)
{
    GPUDevice_Vulkan* D      = (GPUDevice_Vulkan*)pool->queue->pDevice;
    GPUCommandPool_Vulkan* P = (GPUCommandPool_Vulkan*)pool;
    VkResult r = D->mVkDeviceTable.vkResetCommandPool(D->pDevice, P->pPool, 0);
    if (r != VK_SUCCESS) assert(0);
}

GPUCommandBufferID GPUCreateCommandBuffer_Vulkan(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc)
{
    GPUDevice_Vulkan* D        = (GPUDevice_Vulkan*)pool->queue->pDevice;
    GPUCommandPool_Vulkan* P   = (GPUCommandPool_Vulkan*)pool;
    GPUCommandBuffer_Vulkan* B = (GPUCommandBuffer_Vulkan*)_aligned_malloc(sizeof(GPUCommandBuffer_Vulkan), _alignof(GPUCommandBuffer_Vulkan));
    memset(B, 0, sizeof(GPUCommandBuffer_Vulkan));
    B->type = pool->queue->queueType;

    VkCommandBufferAllocateInfo info{};
    info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool        = P->pPool;
    info.level              = desc->isSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandBufferCount = 1;

    VkResult rs = D->mVkDeviceTable.vkAllocateCommandBuffers(D->pDevice, &info, &B->pVkCmd);
    assert(rs == VK_SUCCESS);

    return &B->super;
}

void GPUFreeCommandBuffer_Vulkan(GPUCommandBufferID cmd)
{
    GPUDevice_Vulkan* D         = (GPUDevice_Vulkan*)cmd->device;
    GPUCommandPool_Vulkan* pool = (GPUCommandPool_Vulkan*)cmd->pool;
    GPUCommandBuffer_Vulkan* B  = (GPUCommandBuffer_Vulkan*)cmd;
    D->mVkDeviceTable.vkFreeCommandBuffers(D->pDevice, pool->pPool, 1, &B->pVkCmd);
    _aligned_free(B);
}

void GPUCmdBegin_Vulkan(GPUCommandBufferID cmdBuffer)
{
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)cmdBuffer->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)cmdBuffer;

    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    assert(D->mVkDeviceTable.vkBeginCommandBuffer(CMD->pVkCmd, &info) == VK_SUCCESS);
    CMD->pLayout = VK_NULL_HANDLE;
}

void GPUCmdEnd_Vulkan(GPUCommandBufferID cmdBuffer)
{
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)cmdBuffer->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)cmdBuffer;
    assert(D->mVkDeviceTable.vkEndCommandBuffer(CMD->pVkCmd) == VK_SUCCESS);
}

void GPUCmdResourceBarrier_Vulkan(GPUCommandBufferID cmd, const GPUResourceBarrierDescriptor* desc)
{
    GPUCommandBuffer_Vulkan* Cmd = (GPUCommandBuffer_Vulkan*)cmd;
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)cmd->device;
    GPUAdapter_Vulkan* A = (GPUAdapter_Vulkan*)cmd->device->pAdapter;
    VkAccessFlags srcAccessFlags = 0;
    VkAccessFlags dstAccessFlags = 0;

    DECLEAR_ZERO_VAL(VkBufferMemoryBarrier, BBs, desc->buffer_barriers_count);
    uint32_t bufferBarrierCount = 0;
    for (uint32_t i = 0; i < desc->buffer_barriers_count; i++)
    {
        const GPUBufferBarrier* buffer_barrier = &desc->buffer_barriers[i];
        GPUBuffer_Vulkan* B = (GPUBuffer_Vulkan*)buffer_barrier->buffer;
        VkBufferMemoryBarrier* pBufferBarrier = NULL;

        if (GPU_RESOURCE_STATE_UNORDERED_ACCESS == buffer_barrier->src_state &&
            GPU_RESOURCE_STATE_UNORDERED_ACCESS == buffer_barrier->dst_state)
        {
            pBufferBarrier = &BBs[bufferBarrierCount++];                     //-V522
            pBufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER; //-V522
            pBufferBarrier->pNext = NULL;

            pBufferBarrier->srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            pBufferBarrier->dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        }
        else
        {
            pBufferBarrier = &BBs[bufferBarrierCount++];
            pBufferBarrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            pBufferBarrier->pNext = NULL;

            pBufferBarrier->srcAccessMask = VulkanUtil_ResourceStateToVkAccessFlags(buffer_barrier->src_state);
            pBufferBarrier->dstAccessMask = VulkanUtil_ResourceStateToVkAccessFlags(buffer_barrier->dst_state);
        }

        if (pBufferBarrier)
        {
            pBufferBarrier->buffer = B->pVkBuffer;
            pBufferBarrier->size = VK_WHOLE_SIZE;
            pBufferBarrier->offset = 0;

            if (buffer_barrier->queue_acquire)
            {
                pBufferBarrier->dstQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[Cmd->type];
                pBufferBarrier->srcQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[buffer_barrier->queue_type];
            }
            else if (buffer_barrier->queue_release)
            {
                pBufferBarrier->srcQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[Cmd->type];
                pBufferBarrier->dstQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[buffer_barrier->queue_type];
            }
            else
            {
                pBufferBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                pBufferBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }
            srcAccessFlags |= pBufferBarrier->srcAccessMask;
            dstAccessFlags |= pBufferBarrier->dstAccessMask;
        }
    }

    DECLEAR_ZERO_VAL(VkImageMemoryBarrier, TBs, desc->texture_barriers_count)
    uint32_t imageBarrierCount = 0;
    for (uint32_t i = 0; i < desc->texture_barriers_count; i++)
    {
        const GPUTextureBarrier* texture_barrier = &desc->texture_barriers[i];
        GPUTexture_Vulkan* T = (GPUTexture_Vulkan*)texture_barrier->texture;
        VkImageMemoryBarrier* pImageBarrier = NULL;
        if (GPU_RESOURCE_STATE_UNORDERED_ACCESS == texture_barrier->src_state &&
            GPU_RESOURCE_STATE_UNORDERED_ACCESS == texture_barrier->dst_state)
        {
            pImageBarrier = &TBs[imageBarrierCount++];
            pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            pImageBarrier->pNext = NULL;

            pImageBarrier->srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            pImageBarrier->dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            pImageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            pImageBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else
        {
            pImageBarrier = &TBs[imageBarrierCount++];
            pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            pImageBarrier->pNext = NULL;

            pImageBarrier->srcAccessMask = VulkanUtil_ResourceStateToVkAccessFlags(texture_barrier->src_state);
            pImageBarrier->dstAccessMask = VulkanUtil_ResourceStateToVkAccessFlags(texture_barrier->dst_state);
            pImageBarrier->oldLayout = VulkanUtil_ResourceStateToImageLayout(texture_barrier->src_state);
            pImageBarrier->newLayout = VulkanUtil_ResourceStateToImageLayout(texture_barrier->dst_state);
        }

        if (pImageBarrier)
        {
            pImageBarrier->image = T->pVkImage;
            pImageBarrier->subresourceRange.aspectMask = (VkImageAspectFlags)T->super.aspectMask;
            pImageBarrier->subresourceRange.baseMipLevel = texture_barrier->subresource_barrier ? texture_barrier->mip_level : 0;
            pImageBarrier->subresourceRange.levelCount = texture_barrier->subresource_barrier ? 1 : VK_REMAINING_MIP_LEVELS;
            pImageBarrier->subresourceRange.baseArrayLayer = texture_barrier->subresource_barrier ? texture_barrier->array_layer : 0;
            pImageBarrier->subresourceRange.layerCount = texture_barrier->subresource_barrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

            if (texture_barrier->queue_acquire &&
                texture_barrier->src_state != GPU_RESOURCE_STATE_UNDEFINED)
            {
                pImageBarrier->dstQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[Cmd->type];
                pImageBarrier->srcQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[texture_barrier->queue_type];
            }
            else if (texture_barrier->queue_release &&
                texture_barrier->src_state != GPU_RESOURCE_STATE_UNDEFINED)
            {
                pImageBarrier->srcQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[Cmd->type];
                pImageBarrier->dstQueueFamilyIndex = (uint32_t)A->queueFamilyIndices[texture_barrier->queue_type];
            }
            else
            {
                pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }

            srcAccessFlags |= pImageBarrier->srcAccessMask;
            dstAccessFlags |= pImageBarrier->dstAccessMask;
        }
    }

    // Commit barriers
    VkPipelineStageFlags srcStageMask =
        VulkanUtil_DeterminePipelineStageFlags(A, srcAccessFlags, (EGPUQueueType)Cmd->type);
    VkPipelineStageFlags dstStageMask =
        VulkanUtil_DeterminePipelineStageFlags(A, dstAccessFlags, (EGPUQueueType)Cmd->type);
    if (bufferBarrierCount || imageBarrierCount)
    {
        D->mVkDeviceTable.vkCmdPipelineBarrier(Cmd->pVkCmd,
            srcStageMask, dstStageMask, 0,
            0, NULL,
            bufferBarrierCount, BBs,
            imageBarrierCount, TBs);
    }
}

GPUFenceID GPUCreateFence_Vulkan(GPUDeviceID device)
{
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)device;
    GPUFence_Vulkan* F  = (GPUFence_Vulkan*)calloc(1, sizeof(GPUFence_Vulkan));

    VkFenceCreateInfo info{};
    info.sType  = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult rs = D->mVkDeviceTable.vkCreateFence(D->pDevice, &info, GLOBAL_VkAllocationCallbacks, &F->pVkFence);
    assert(rs == VK_SUCCESS);
    F->submitted = false;

    return &F->super;
}

void GPUFreeFence_Vulkan(GPUFenceID fence)
{
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)fence->device;
    GPUFence_Vulkan* F  = (GPUFence_Vulkan*)fence;
    D->mVkDeviceTable.vkDestroyFence(D->pDevice, F->pVkFence, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(F);
}

void GPUWaitFences_Vulkan(const GPUFenceID* fences, uint32_t fenceCount)
{
    GPUDevice_Vulkan* D = (GPUDevice_Vulkan*)fences[0]->device;
    DECLEAR_ZERO_VAL(VkFence, vkFences, fenceCount);
    uint32_t validCount = 0;
    for (uint32_t i = 0; i < fenceCount; i++)
    {
        GPUFence_Vulkan* vf = (GPUFence_Vulkan*)fences[i];
        if (vf->submitted) vkFences[validCount++] = vf->pVkFence;
    }
    if (validCount > 0)
    {
        D->mVkDeviceTable.vkWaitForFences(D->pDevice, validCount, vkFences, VK_TRUE, 0xffffffffffffffff);
        D->mVkDeviceTable.vkResetFences(D->pDevice, validCount, vkFences);
    }
    for (uint32_t i = 0; i < fenceCount; i++)
    {
        GPUFence_Vulkan* vf = (GPUFence_Vulkan*)fences[i];
        vf->submitted       = false;
    }
}

EGPUFenceStatus GPUQueryFenceStatus_Vulkan(GPUFenceID fence)
{
    EGPUFenceStatus status = GPU_FENCE_STATUS_COMPLETE;
    GPUFence_Vulkan* F     = (GPUFence_Vulkan*)fence;
    GPUDevice_Vulkan* D    = (GPUDevice_Vulkan*)fence->device;
    if (F->submitted)
    {
        VkResult rs = vkGetFenceStatus(D->pDevice, F->pVkFence);
        status      = rs == VK_SUCCESS ? GPU_FENCE_STATUS_COMPLETE : GPU_FENCE_STATUS_INCOMPLETE;
    }
    else
    {
        status = GPU_FENCE_STATUS_NOTSUBMITTED;
    }
    return status;
}

GPUSemaphoreID GPUCreateSemaphore_Vulkan(GPUDeviceID device)
{
    GPUDevice_Vulkan* D    = (GPUDevice_Vulkan*)device;
    GPUSemaphore_Vulkan* s = (GPUSemaphore_Vulkan*)calloc(1, sizeof(GPUSemaphore_Vulkan));

    VkSemaphoreCreateInfo info{};
    info.sType  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult rs = D->mVkDeviceTable.vkCreateSemaphore(D->pDevice, &info, GLOBAL_VkAllocationCallbacks, &s->pVkSemaphore);
    assert(rs == VK_SUCCESS);
    s->signaled = false;
    
    return &s->super;
}

void GPUFreeSemaphore_Vulkan(GPUSemaphoreID semaphore)
{
    GPUDevice_Vulkan* D    = (GPUDevice_Vulkan*)semaphore->device;
    GPUSemaphore_Vulkan* S = (GPUSemaphore_Vulkan*)semaphore;
    D->mVkDeviceTable.vkDestroySemaphore(D->pDevice, S->pVkSemaphore, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(S);
}

GPURenderPassEncoderID GPUCmdBeginRenderPass_Vulkan(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor* desc)
{
    GPUDevice_Vulkan* D          = (GPUDevice_Vulkan*)cmd->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)cmd;

    uint32_t Width, Height;
    VkRenderPass render_pass = VK_NULL_HANDLE;
    VulkanRenderPassDescriptor r_desc{};
    r_desc.attachmentCount = desc->render_target_count;
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        r_desc.pColorFormat[i]   = desc->color_attachments[i].view->desc.format;
        r_desc.pColorLoadOps[i]  = desc->color_attachments[i].load_action;
        r_desc.pColorStoreOps[i] = desc->color_attachments[i].store_action;
        Width                    = desc->color_attachments[i].view->desc.pTexture->width;
        Height                   = desc->color_attachments[i].view->desc.pTexture->height;
    }
    r_desc.depthFormat    = desc->depth_stencil ? (desc->depth_stencil->view ? desc->depth_stencil->view->desc.format : GPU_FORMAT_UNDEFINED) : GPU_FORMAT_UNDEFINED;
    r_desc.sampleCount    = desc->sample_count;
    r_desc.depthLoadOp    = desc->depth_stencil ? desc->depth_stencil->depth_load_action : GPU_LOAD_ACTION_DONTCARE;
    r_desc.depthStoreOp   = desc->depth_stencil ? desc->depth_stencil->depth_store_action : GPU_STORE_ACTION_STORE;
    r_desc.stencilLoadOp  = desc->depth_stencil ? desc->depth_stencil->stencil_load_action : GPU_LOAD_ACTION_DONTCARE;
    r_desc.stencilStoreOp = desc->depth_stencil ? desc->depth_stencil->stencil_store_action : GPU_STORE_ACTION_STORE;
    FindOrCreateRenderPass(D, &r_desc, &render_pass);

    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VulkanFramebufferDesriptor fb_desc{};
    fb_desc.pRenderPass     = render_pass;
    fb_desc.width           = Width;
    fb_desc.height          = Height;
    fb_desc.layers          = 1;
    uint32_t idx            = 0;
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        GPUTextureView_Vulkan* view = (GPUTextureView_Vulkan*)desc->color_attachments[i].view;
        fb_desc.pImageViews[idx]    = view->pVkRTVDSVDescriptor;
        fb_desc.layers              = view->super.desc.arrayLayerCount;
        fb_desc.attachmentCount    += 1;
        idx++;
    }
    if (desc->depth_stencil != VK_NULL_HANDLE && desc->depth_stencil->view != VK_NULL_HANDLE)
    {
        GPUTextureView_Vulkan* view = (GPUTextureView_Vulkan*)desc->depth_stencil->view;
        fb_desc.pImageViews[idx]    = view->pVkRTVDSVDescriptor;
        fb_desc.layers              = view->super.desc.arrayLayerCount;
        fb_desc.attachmentCount    += 1;
        idx++;
    }
    if (desc->render_target_count)
        assert(fb_desc.layers == 1 && "MRT pass supports only one layer!");
    FindOrCreateFrameBuffer(D, &fb_desc, &framebuffer);

    VkRect2D renderArea{};
    renderArea.extent.width  = Width;
    renderArea.extent.height = Height;
    renderArea.offset.x      = 0;
    renderArea.offset.y      = 0;

    VkClearValue pClearValues[GPU_MAX_MRT_COUNT + 1] = {};
    idx                                              = 0;
    for (uint32_t i = 0; i < desc->render_target_count; i++)
    {
        const GPUClearValue* clearValue    = &desc->color_attachments[i].clear_color;
        pClearValues[idx].color.float32[0] = clearValue->r;
        pClearValues[idx].color.float32[1] = clearValue->g;
        pClearValues[idx].color.float32[2] = clearValue->b;
        pClearValues[idx].color.float32[3] = clearValue->a;
        idx++;
    }
    if (desc->depth_stencil != VK_NULL_HANDLE && desc->depth_stencil->view != VK_NULL_HANDLE)
    {
        pClearValues[idx].depthStencil.depth   = desc->depth_stencil->clear_depth;
        pClearValues[idx].depthStencil.stencil = desc->depth_stencil->clear_stencil;
        idx++;
    }

    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass      = render_pass;
    beginInfo.framebuffer     = framebuffer;
    beginInfo.renderArea      = renderArea;
    beginInfo.clearValueCount = idx;
    beginInfo.pClearValues    = pClearValues;

    D->mVkDeviceTable.vkCmdBeginRenderPass(CMD->pVkCmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    CMD->pPass = render_pass;
    return (GPURenderPassEncoderID)cmd;
}

void GPUCmdEndRenderPass_Vulkan(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder)
{
    GPUDevice_Vulkan* D          = (GPUDevice_Vulkan*)cmd->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)cmd;
    D->mVkDeviceTable.vkCmdEndRenderPass(CMD->pVkCmd);
    CMD->pPass = VK_NULL_HANDLE;
}

void GPURenderEncoderSetViewport_Vulkan(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    GPUDevice_Vulkan* D          = (GPUDevice_Vulkan*)encoder->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)encoder;
    VkViewport viewport{};
    viewport.x        = x;
    viewport.y        = y;
    viewport.width    = width;
    viewport.height   = height;
    viewport.minDepth = min_depth;
    viewport.maxDepth = max_depth;
    D->mVkDeviceTable.vkCmdSetViewport(CMD->pVkCmd, 0, 1, &viewport);
}

void GPURenderEncoderSetScissor_Vulkan(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    GPUDevice_Vulkan* D          = (GPUDevice_Vulkan*)encoder->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)encoder;
    VkRect2D scissor{};
    scissor.offset = { (int32_t)x, (int32_t)y };
    scissor.extent = { width, height };
    D->mVkDeviceTable.vkCmdSetScissor(CMD->pVkCmd, 0, 1, &scissor);
}

void GPURenderEncoderBindPipeline_Vulkan(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline)
{
    GPUDevice_Vulkan* D          = (GPUDevice_Vulkan*)encoder->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)encoder;
    GPURenderPipeline_Vulkan* PP = (GPURenderPipeline_Vulkan*)pipeline;
    D->mVkDeviceTable.vkCmdBindPipeline(CMD->pVkCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, PP->pPipeline);
}

void GPURenderEncoderDraw_Vulkan(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    GPUDevice_Vulkan* D          = (GPUDevice_Vulkan*)encoder->device;
    GPUCommandBuffer_Vulkan* CMD = (GPUCommandBuffer_Vulkan*)encoder;
    D->mVkDeviceTable.vkCmdDraw(CMD->pVkCmd, vertex_count, 1, first_vertex, 0);
}
