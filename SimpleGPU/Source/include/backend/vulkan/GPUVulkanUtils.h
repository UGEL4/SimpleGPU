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
    void VulkanUtil_RecordAdaptorDetail(GPUAdapter_Vulkan* pAdapter);

    uint32_t VulkanUtil_BitSizeOfBlock(EGPUFormat format);
    VkSampleCountFlagBits VulkanUtil_SampleCountToVk(EGPUSampleCount sampleCount);
    VkPrimitiveTopology VulkanUtil_PrimitiveTopologyToVk(EGPUPrimitiveTopology topology);
    VkCompareOp VulkanUtil_CompareOpToVk(EGPUCompareMode compareMode);
    VkStencilOp VulkanUtil_StencilOpToVk(EGPUStencilOp op);

    void VulkanUtil_InitializeShaderReflection(GPUDeviceID device, GPUShaderLibrary_Vulkan* S, const struct GPUShaderLibraryDescriptor* desc);
    void VulkanUtil_FreeShaderReflection(GPUShaderLibrary_Vulkan* S);

    VkRenderPass VulkanUtil_RenderPassTableTryFind(struct GPUVkPassTable* table, const struct VulkanRenderPassDescriptor* desc);
    void VulkanUtil_RenderPassTableAdd(struct GPUVkPassTable* table, const struct VulkanRenderPassDescriptor* desc, VkRenderPass pass);
    void VulkanUtil_ConsumeDescriptorSets(VkUtil_DescriptorPool* pool, const VkDescriptorSetLayout* pLayouts, VkDescriptorSet* pSets, uint32_t setsNum);
    void VulkanUtil_ReturnDescriptorSets(struct VkUtil_DescriptorPool* pPool, VkDescriptorSet* pSets, uint32_t setsNum);

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

    inline static VkAccessFlags VulkanUtil_ResourceStateToVkAccessFlags(EGPUResourceState state)
    {
        VkAccessFlags ret = 0;
        if (state & GPU_RESOURCE_STATE_COPY_SOURCE)
            ret |= VK_ACCESS_TRANSFER_READ_BIT;
        if (state & GPU_RESOURCE_STATE_COPY_DEST)
            ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
        if (state & GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            ret |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        if (state & GPU_RESOURCE_STATE_INDEX_BUFFER)
            ret |= VK_ACCESS_INDEX_READ_BIT;
        if (state & GPU_RESOURCE_STATE_UNORDERED_ACCESS)
            ret |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        if (state & GPU_RESOURCE_STATE_INDIRECT_ARGUMENT)
            ret |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        if (state & GPU_RESOURCE_STATE_RENDER_TARGET)
            ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if (state & GPU_RESOURCE_STATE_RESOLVE_DEST)
            ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if (state & GPU_RESOURCE_STATE_DEPTH_WRITE)
            ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        if (state & GPU_RESOURCE_STATE_SHADER_RESOURCE)
            ret |= VK_ACCESS_SHADER_READ_BIT;
        if (state & GPU_RESOURCE_STATE_PRESENT)
        {
            ret |= VK_ACCESS_MEMORY_READ_BIT;
        }
#ifdef ENABLE_RAYTRACING
        if (state & GPU_RESOURCE_STATE_ACCELERATION_STRUCTURE)
            ret |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
#endif
        return ret;
    }

    inline static VkImageLayout VulkanUtil_ResourceStateToImageLayout(EGPUResourceState usage)
    {
        if (usage & GPU_RESOURCE_STATE_COPY_SOURCE)
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        if (usage & GPU_RESOURCE_STATE_COPY_DEST)
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        if (usage & GPU_RESOURCE_STATE_RENDER_TARGET)
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (usage & GPU_RESOURCE_STATE_RESOLVE_DEST)
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (usage & GPU_RESOURCE_STATE_DEPTH_WRITE)
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        if (usage & GPU_RESOURCE_STATE_UNORDERED_ACCESS)
            return VK_IMAGE_LAYOUT_GENERAL;

        if (usage & GPU_RESOURCE_STATE_SHADER_RESOURCE)
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if (usage & GPU_RESOURCE_STATE_PRESENT)
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        if (usage == GPU_RESOURCE_STATE_COMMON)
            return VK_IMAGE_LAYOUT_GENERAL;

        if (usage == GPU_RESOURCE_STATE_SHADING_RATE_SOURCE)
            return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;

        return VK_IMAGE_LAYOUT_UNDEFINED;
    }

    inline static VkPipelineStageFlags VulkanUtil_DeterminePipelineStageFlags(GPUAdapter_Vulkan* A, VkAccessFlags accessFlags, EGPUQueueType queue_type)
    {
        VkPipelineStageFlags flags = 0;

        switch (queue_type)
        {
        case GPU_QUEUE_TYPE_GRAPHICS:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                /*if (A->adapterDetail.support_geom_shader)
                {
                    flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                }
                if (A->adapterDetail.support_tessellation)
                {
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                }*/
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
#ifdef ENABLE_RAYTRACING
                if (pRenderer->mVulkan.mRaytracingExtension)
                {
                    flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV;
                }
#endif
            }
            if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        }
        case GPU_QUEUE_TYPE_COMPUTE:
        {
            if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
                (accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
                (accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
                (accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            break;
        }
        case GPU_QUEUE_TYPE_TRANSFER: return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        default: break;
        }
        // Compatible with both compute and graphics queues
        if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
            flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

        if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
            flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

        if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
            flags |= VK_PIPELINE_STAGE_HOST_BIT;

        if (flags == 0)
            flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        return flags;
    }

    inline static VkImageAspectFlags VulkanUtil_DeterminAspectMask(VkFormat format, bool includeStencilBit)
    {
        VkImageAspectFlags result = 0;
        switch (format)
        {
            // Depth
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            break;
            // Stencil
            case VK_FORMAT_S8_UINT:
            result = VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
            // Depth/stencil
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
            result = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (includeStencilBit)
                result |= VK_IMAGE_ASPECT_STENCIL_BIT;
            break;
            // Assume everything else is Color
            default:
            result = VK_IMAGE_ASPECT_COLOR_BIT;
            break;
        }
        return result;
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
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        /************************************************************************/
        // Descriptor Update Template Extension for efficient descriptor set updates
        /************************************************************************/
        VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME
        //VK_KHR_MAINTENANCE1_EXTENSION_NAME // Vulkan NDC fixed, vk 1.0
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

    inline static VkBufferUsageFlags VulkanUtil_DescriptorTypesToBufferUsage(GPUResourceTypes descriptors, bool texel)
    {
        VkBufferUsageFlags result = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        if (descriptors & GPU_RESOURCE_TYPE_UNIFORM_BUFFER)
        {
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }
        if (descriptors & GPU_RESOURCE_TYPE_RW_BUFFER)
        {
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            if (texel) result |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        }
        if (descriptors & GPU_RESOURCE_TYPE_BUFFER)
        {
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            if (texel) result |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        }
        if (descriptors & GPU_RESOURCE_TYPE_INDEX_BUFFER)
        {
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if (descriptors & GPU_RESOURCE_TYPE_VERTEX_BUFFER)
        {
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }
        if (descriptors & GPU_RESOURCE_TYPE_INDIRECT_BUFFER)
        {
            result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }
#ifdef ENABLE_RAYTRACING
        if (descriptors & CGPU_RESOURCE_TYPE_RAY_TRACING)
        {
            result |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        }
#endif
        return result;
    }

    inline static VkBufferUsageFlags VulkanUtil_DescriptorTypesToImageUsage(GPUResourceTypes descriptors)
    {
        VkImageUsageFlags result = 0;
        if (GPU_RESOURCE_TYPE_TEXTURE == (descriptors & GPU_RESOURCE_TYPE_TEXTURE))
            result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (GPU_RESOURCE_TYPE_RW_TEXTURE == (descriptors & GPU_RESOURCE_TYPE_RW_TEXTURE))
            result |= VK_IMAGE_USAGE_STORAGE_BIT;
        return result;
    }

    static VkFilter VulkanUtil_TranslateFilterType(EGPUFilterType type)
    {
        switch (type)
        {
            case GPU_FILTER_TYPE_LINEAR:
                return VK_FILTER_LINEAR;
            case GPU_FILTER_TYPE_NEAREST:
                return VK_FILTER_NEAREST;
            default:
                return VK_FILTER_LINEAR;
        }
    }

    static VkSamplerMipmapMode VulkanUtil_TranslateMipMapMode(EGPUMipMapMode mode)
    {
        switch (mode)
        {
            case GPU_MIPMAP_MODE_LINEAR:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            case GPU_MIPMAP_MODE_NEAREST:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }
    
    static VkSamplerAddressMode VulkanUtil_TranslateAddressMode(EGPUAddressMode mode)
    {
        switch (mode)
        {
            case GPU_ADDRESS_MODE_MIRROR:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case GPU_ADDRESS_MODE_REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case GPU_ADDRESS_MODE_CLAMP_TO_EDGE:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case GPU_ADDRESS_MODE_CLAMP_TO_BORDER:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            default:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    static const VkCompareOp gVkCompareOpTranslator[GPU_CMP_COUNT] = {
        VK_COMPARE_OP_NEVER,
        VK_COMPARE_OP_LESS,
        VK_COMPARE_OP_EQUAL,
        VK_COMPARE_OP_LESS_OR_EQUAL,
        VK_COMPARE_OP_GREATER,
        VK_COMPARE_OP_NOT_EQUAL,
        VK_COMPARE_OP_GREATER_OR_EQUAL,
        VK_COMPARE_OP_ALWAYS,
    };

    static VkFormatFeatureFlags VulkanUtil_ImageUsageToFormatFeatures(VkImageUsageFlags usage)
    {
        VkFormatFeatureFlags result = (VkFormatFeatureFlags)0;
        if (VK_IMAGE_USAGE_SAMPLED_BIT == (usage & VK_IMAGE_USAGE_SAMPLED_BIT))
        {
            result |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
        }
        if (VK_IMAGE_USAGE_STORAGE_BIT == (usage & VK_IMAGE_USAGE_STORAGE_BIT))
        {
            result |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
        }
        if (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT == (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
        {
            result |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
        }
        if (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT == (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            result |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        return result;
    }

#ifdef __cplusplus
}
#endif // __cplusplus