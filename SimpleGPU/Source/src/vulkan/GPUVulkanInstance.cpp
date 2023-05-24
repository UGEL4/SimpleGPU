#include "backend/vulkan/GPUVulkan.h"
#include <stdlib.h>
#include <cstring>
#include <assert.h>
#include <vector>
#include <algorithm>
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
    .CreateInstance      = &CreateInstance_Vulkan,
    .FreeInstance        = &FreeInstance_Vllkan,
    .EnumerateAdapters   = &EnumerateAdapters_Vulkan,
    .CreateDevice        = &CreateDevice_Vulkan,
    .FreeDevice          = &FreeDevice_Vulkan,
    .GetQueue            = &GetQueue_Vulkan,
    .CreateSwapchain     = &GPUCreateSwapchain_Vulkan,
    .FreeSwapchain       = &GPUFreeSwapchain_Vulkan,
    .CreateTextureView   = &GPUCreateTextureView_Vulkan,
    .FreeTextureView     = &GPUFreeTextureView_Vulkan,
    .CreateShaderLibrary = &GPUCreateShaderLibrary_Vulkan,
    .FreeShaderLibrary   = &GPUFreeShaderLibrary_Vulkan
};
const GPUProcTable* GPUVulkanProcTable()
{
    return &vkTable;
}

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
    appInfo.apiVersion         = VK_API_VERSION_1_1;
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

    return &(pDevice->spuer);
}

void FreeDevice_Vulkan(GPUDeviceID pDevice)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;
    vkDestroyDevice(pVkDevice->pDevice, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(pVkDevice);
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
        // pTex[i].super.aspectMask = VkUtil_DeterminAspectMask(Ts[i].super.format, false);
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

GPUShaderLibraryID GPUCreateShaderLibrary_Vulkan(GPUDeviceID pDevice, GPUShaderLibraryDescriptor* pDesc)
{
    GPUDevice_Vulkan* pVkDevice = (GPUDevice_Vulkan*)pDevice;

    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = pDesc->codeSize;
    info.pCode    = pDesc->code;

    GPUShaderLibrary_Vulkan* pShader = (GPUShaderLibrary_Vulkan*)malloc(sizeof(GPUShaderLibrary_Vulkan));
    if (pVkDevice->mVkDeviceTable.vkCreateShaderModule(pVkDevice->pDevice, &info, GLOBAL_VkAllocationCallbacks, &pShader->pShader) != VK_SUCCESS)
    {
        assert(0);
    }

    return &pShader->super;
}

void GPUFreeShaderLibrary_Vulkan(GPUShaderLibraryID pShader)
{
    GPUShaderLibrary_Vulkan* pVkShader = (GPUShaderLibrary_Vulkan*)pShader;
    GPUDevice_Vulkan* pVkDevice        = (GPUDevice_Vulkan*)pShader->pDevice;
    pVkDevice->mVkDeviceTable.vkDestroyShaderModule(pVkDevice->pDevice, pVkShader->pShader, GLOBAL_VkAllocationCallbacks);
    GPU_SAFE_FREE(pVkShader);
}

