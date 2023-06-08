#pragma once
#include <cstdint>
#include "backend/vulkan/GPUVulkan.h"
#include <assert.h>

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

    void VulkanUtil_InitializeShaderReflection(GPUDeviceID device, GPUShaderLibrary_Vulkan* S, const struct GPUShaderLibraryDescriptor* desc);
    void VulkanUtil_FreeShaderReflection(GPUShaderLibrary_Vulkan* S);

    VkRenderPass VulkanUtil_RenderPassTableTryFind(struct GPUVkPassTable* table, const struct VulkanRenderPassDescriptor* desc);
    void VulkanUtil_RenderPassTableAdd(struct GPUVkPassTable* table, const struct VulkanRenderPassDescriptor* desc, VkRenderPass pass);

    inline static VkShaderStageFlags VulkanUtil_TranslateShaderUsages(GPUShaderStages shader_stages)
    {
        VkShaderStageFlags result = 0;
        if (GPU_SHADER_STAGE_ALL_GRAPHICS == (shader_stages & GPU_SHADER_STAGE_ALL_GRAPHICS))
        {
            result = VK_SHADER_STAGE_ALL_GRAPHICS;
        }
        else
        {
            if (GPU_SHADER_STAGE_VERT == (shader_stages & GPU_SHADER_STAGE_VERT))
            {
                result |= VK_SHADER_STAGE_VERTEX_BIT;
            }
            if (GPU_SHADER_STAGE_TESC == (shader_stages & GPU_SHADER_STAGE_TESC))
            {
                result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            }
            if (GPU_SHADER_STAGE_TESE == (shader_stages & GPU_SHADER_STAGE_TESE))
            {
                result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            }
            if (GPU_SHADER_STAGE_GEOM == (shader_stages & GPU_SHADER_STAGE_GEOM))
            {
                result |= VK_SHADER_STAGE_GEOMETRY_BIT;
            }
            if (GPU_SHADER_STAGE_FRAG == (shader_stages & GPU_SHADER_STAGE_FRAG))
            {
                result |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            if (GPU_SHADER_STAGE_COMPUTE == (shader_stages & GPU_SHADER_STAGE_COMPUTE))
            {
                result |= VK_SHADER_STAGE_COMPUTE_BIT;
            }
        }
        return result;
    }

    inline static VkDescriptorType VulkanUtil_TranslateResourceType(EGPUResourceType type)
    {
        switch (type)
        {
            case GPU_RESOURCE_TYPE_NONE:
                assert(0 && "Invalid DescriptorInfo Type");
                return VK_DESCRIPTOR_TYPE_MAX_ENUM;
            case GPU_RESOURCE_TYPE_SAMPLER:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
            case GPU_RESOURCE_TYPE_TEXTURE:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case GPU_RESOURCE_TYPE_UNIFORM_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case GPU_RESOURCE_TYPE_RW_TEXTURE:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case GPU_RESOURCE_TYPE_BUFFER:
            case GPU_RESOURCE_TYPE_RW_BUFFER:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case GPU_RESOURCE_TYPE_INPUT_ATTACHMENT:
                return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            case GPU_RESOURCE_TYPE_TEXEL_BUFFER:
                return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER:
                return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case GPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#ifdef ENABLE_RAYTRACING
            case CGPU_RESOURCE_TYPE_RAY_TRACING:
                return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
#endif
            default:
                assert(0 && "Invalid DescriptorInfo Type");
                return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

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

    #define GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE (VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT + 1)
    static const VkDescriptorPoolSize gDescriptorPoolSizes[GPU_VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 8192 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8192 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 },
    };

    static const VkPipelineBindPoint gPipelineBindPoint[GPU_PIPELINE_TYPE_COUNT] = {
        VK_PIPELINE_BIND_POINT_MAX_ENUM,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
#ifdef ENABLE_RAYTRACING
        VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
#endif
    };

#ifdef __cplusplus
}
#endif // __cplusplus