#include "backend/vulkan/GPUVulkanUtils.h"
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <assert.h>
#include "shader-reflections/spirv/spirv_reflect.h"
#include "api.h"

void VulkanUtil_SelectValidationLayers(GPUInstance_Vulkan* pInstance, const char** instanceLayers, uint32_t layersCount)
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, VK_NULL_HANDLE);

    if (count > 0)
    {
        pInstance->pLayerProperties = (VkLayerProperties*)malloc(layersCount * sizeof(VkLayerProperties));
        pInstance->pLayerNames      = (const char**)malloc(layersCount * sizeof(const char*));

        // std::vector<VkLayerProperties> props(count);
        DECLEAR_ZERO_VAL(VkLayerProperties, layerProps, count);
        vkEnumerateInstanceLayerProperties(&count, layerProps);
        uint32_t filledCount = 0;
        for (uint32_t i = 0; i < layersCount; i++)
        {
            for (uint32_t j = 0; j < count; j++)
            {
                if (strcmp(instanceLayers[i], layerProps[j].layerName) == 0)
                {
                    pInstance->pLayerProperties[filledCount] = layerProps[j];
                    pInstance->pLayerNames[filledCount]      = pInstance->pLayerProperties[filledCount].layerName;
                    filledCount++;
                    break;
                }
            }
        }
        pInstance->layersCount = filledCount;
    }
}

void VulkanUtil_SelectInstanceExtensions(struct GPUInstance_Vulkan* pInstance, const char** instanceExtensions, uint32_t extensionCount)
{
    const char* layerName = VK_NULL_HANDLE;
    uint32_t count        = 0;
    vkEnumerateInstanceExtensionProperties(layerName, &count, VK_NULL_HANDLE);
    if (count > 0)
    {
        pInstance->pExtensonProperties = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
        pInstance->pExtensionNames     = (const char**)malloc(extensionCount * sizeof(const char*));

        DECLEAR_ZERO_VAL(VkExtensionProperties, extensions, count);
        vkEnumerateInstanceExtensionProperties(layerName, &count, extensions);
        uint32_t filledCount = 0;
        for (uint32_t i = 0; i < extensionCount; i++)
        {
            for (uint32_t j = 0; j < count; j++)
            {
                if (strcmp(instanceExtensions[i], extensions[j].extensionName) == 0)
                {
                    pInstance->pExtensonProperties[filledCount] = extensions[j];
                    pInstance->pExtensionNames[filledCount]     = pInstance->pExtensonProperties[filledCount].extensionName;
                    filledCount++;
                    break;
                }
            }
        }
        pInstance->extensionCount = filledCount;
    }
}

void VulkanUtil_EnableValidationLayers(struct GPUInstance_Vulkan* pInstance, const struct VkDebugUtilsMessengerCreateInfoEXT* pMessengerCreateInfo)
{
    if (pInstance->debugUtils)
    {
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{};
        messengerInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        messengerInfo.pfnUserCallback = VulkanUtil_DebugUtilsCallback;

        const VkDebugUtilsMessengerCreateInfoEXT* ptr = (pMessengerCreateInfo != VK_NULL_HANDLE) ? pMessengerCreateInfo : &messengerInfo;
        assert(vkCreateDebugUtilsMessengerEXT && "load vkCreateDebugUtilsMessengerEXT failed!");
        VkResult result = vkCreateDebugUtilsMessengerEXT(pInstance->pInstance, ptr, GLOBAL_VkAllocationCallbacks, &pInstance->pDebugUtils);
        if (result != VK_SUCCESS)
        {
            assert(0);
        }
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanUtil_DebugUtilsCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                              void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            std::cout << "Vulkan validation verbose layer:\n"
                      << pCallbackData->pMessage << std::endl;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            std::cout << "Vulkan validation info layer:\n"
                      << pCallbackData->pMessage << std::endl;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cout << "Vulkan validation warning layer:\n"
                      << pCallbackData->pMessage << std::endl;
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cerr << "Vulkan validation error layer:\n"
                      << pCallbackData->pMessage << std::endl;
            break;
        default:
            return VK_TRUE;
    }
    return VK_FALSE;
}

void VulkanUtil_QueryAllAdapters(struct GPUInstance_Vulkan* pInstance, const char** ppExtensions, uint32_t extensionsCount)
{
    // uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(pInstance->pInstance, &pInstance->adapterCount, VK_NULL_HANDLE);
    if (pInstance->adapterCount > 0)
    {
        pInstance->pAdapters = (GPUAdapter_Vulkan*)malloc(pInstance->adapterCount * sizeof(GPUAdapter_Vulkan));
        DECLEAR_ZERO_VAL(VkPhysicalDevice, deviceArray, pInstance->adapterCount);
        vkEnumeratePhysicalDevices(pInstance->pInstance, &pInstance->adapterCount, deviceArray);
        for (uint32_t i = 0; i < pInstance->adapterCount; i++)
        {
            GPUAdapter_Vulkan* pVkAdapter = &pInstance->pAdapters[i];
            pVkAdapter->pPhysicalDevice   = deviceArray[i];
            for (uint32_t q = 0; q < GPU_QUEUE_TYPE_COUNT; q++)
            {
                pVkAdapter->queueFamilyIndices[q] = -1;
            }

            // properties
            pVkAdapter->physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            {
                void** ppNext                        = &pVkAdapter->physicalDeviceProperties.pNext;
                pVkAdapter->subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
                pVkAdapter->subgroupProperties.pNext = VK_NULL_HANDLE;
                *ppNext                              = &pVkAdapter->subgroupProperties;
                ppNext                               = &pVkAdapter->subgroupProperties.pNext;
            }
            assert(vkGetPhysicalDeviceProperties2KHR && "load vkGetPhysicalDeviceProperties2KHR failed!");
            vkGetPhysicalDeviceProperties2KHR(pVkAdapter->pPhysicalDevice, &pVkAdapter->physicalDeviceProperties);

            // feature
            pVkAdapter->physicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            {
                void** ppNext = &pVkAdapter->physicalDeviceFeatures.pNext;
                *ppNext       = VK_NULL_HANDLE;
            }
            vkGetPhysicalDeviceFeatures2(pVkAdapter->pPhysicalDevice, &pVkAdapter->physicalDeviceFeatures);

            // extensions
            VulkanUtil_SelectPhysicalDeviceExtensions(pVkAdapter, ppExtensions, extensionsCount);
            // queue family index
            VulkanUtil_SelectQueueFamilyIndex(pVkAdapter);
            // format
            VulkanUtil_EnumFormatSupport(pVkAdapter);
        }
    }
    else
    {
        assert(0);
    }
}

void VulkanUtil_SelectQueueFamilyIndex(GPUAdapter_Vulkan* pAdapter)
{
    vkGetPhysicalDeviceQueueFamilyProperties(pAdapter->pPhysicalDevice, &pAdapter->queueFamiliesCount, VK_NULL_HANDLE);
    if (pAdapter->queueFamiliesCount > 0)
    {
        pAdapter->pQueueFamilyProperties = (VkQueueFamilyProperties*)malloc(pAdapter->queueFamiliesCount * sizeof(VkQueueFamilyProperties));
        vkGetPhysicalDeviceQueueFamilyProperties(pAdapter->pPhysicalDevice, &pAdapter->queueFamiliesCount, pAdapter->pQueueFamilyProperties);
        for (uint32_t i = 0; i < pAdapter->queueFamiliesCount; i++)
        {
            if (pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_GRAPHICS] == -1 &&
                pAdapter->pQueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_GRAPHICS] = i;
            }
            else if (pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_COMPUTE] == -1 &&
                     pAdapter->pQueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_COMPUTE] = i;
            }
            else if (pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_TRANSFER] == -1 &&
                     pAdapter->pQueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                pAdapter->queueFamilyIndices[GPU_QUEUE_TYPE_TRANSFER] = i;
            }
        }
    }
    else
    {
        assert(0);
    }
}

void VulkanUtil_SelectPhysicalDeviceExtensions(GPUAdapter_Vulkan* pAdapter, const char** ppExtensions, uint32_t extensionsCount)
{
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(pAdapter->pPhysicalDevice, VK_NULL_HANDLE, &count, VK_NULL_HANDLE);
    if (count > 0)
    {
        pAdapter->pExtensionProps  = (VkExtensionProperties*)malloc(extensionsCount * sizeof(VkExtensionProperties));
        pAdapter->ppExtensionsName = (const char**)malloc(extensionsCount * sizeof(const char*));
        DECLEAR_ZERO_VAL(VkExtensionProperties, props, count);
        vkEnumerateDeviceExtensionProperties(pAdapter->pPhysicalDevice, VK_NULL_HANDLE, &count, props);
        uint32_t filledCount = 0;
        for (uint32_t i = 0; i < extensionsCount; i++)
        {
            for (uint32_t j = 0; j < count; j++)
            {
                if (strcmp(ppExtensions[i], props[j].extensionName) == 0)
                {
                    pAdapter->pExtensionProps[filledCount]  = props[j];
                    pAdapter->ppExtensionsName[filledCount] = pAdapter->pExtensionProps[filledCount].extensionName;
                    filledCount++;
                    break;
                }
            }
        }
        pAdapter->extensionsCount = filledCount;
    }
}

void VulkanUtil_EnumFormatSupport(GPUAdapter_Vulkan* pAdapter)
{
    GPUAdapterDetail* pAdapterDetail = &pAdapter->adapterDetail;
    for (uint32_t i = 0; i < EGPUFormat::GPU_FORMT_COUNT; i++)
    {
        VkFormatProperties formatProps;
        VkFormat fmt = GPUFormatToVulkanFormat((EGPUFormat)i);
        if (fmt == VK_FORMAT_UNDEFINED)
        {
            continue;
        }
        vkGetPhysicalDeviceFormatProperties(pAdapter->pPhysicalDevice, fmt, &formatProps);
    }
}

uint32_t VulkanUtil_BitSizeOfBlock(EGPUFormat format)
{
    switch (format)
    {
        case EGPUFormat::GPU_FORMAT_R16_UINT: return 2;
        case EGPUFormat::GPU_FORMAT_R32_UINT: return 4;
        case EGPUFormat::GPU_FORMAT_R32_SFLOAT: return 4;
    }
    return 0;
}

VkSampleCountFlagBits VulkanUtil_SampleCountToVk(EGPUSampleCount sampleCount)
{
    VkSampleCountFlagBits result = VK_SAMPLE_COUNT_1_BIT;
    switch (sampleCount)
    {
        case GPU_SAMPLE_COUNT_1:
            result = VK_SAMPLE_COUNT_1_BIT;
            break;
        case GPU_SAMPLE_COUNT_2:
            result = VK_SAMPLE_COUNT_2_BIT;
            break;
        case GPU_SAMPLE_COUNT_4:
            result = VK_SAMPLE_COUNT_4_BIT;
            break;
        case GPU_SAMPLE_COUNT_8:
            result = VK_SAMPLE_COUNT_8_BIT;
            break;
        case GPU_SAMPLE_COUNT_16:
            result = VK_SAMPLE_COUNT_16_BIT;
            break;
        default:
            return result;
    }
    return result;
}

VkPrimitiveTopology VulkanUtil_PrimitiveTopologyToVk(EGPUPrimitiveTopology topology)
{
    VkPrimitiveTopology vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (topology)
    {
        case GPU_PRIM_TOPO_POINT_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            break;
        case GPU_PRIM_TOPO_LINE_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            break;
        case GPU_PRIM_TOPO_LINE_STRIP:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            break;
        case GPU_PRIM_TOPO_TRI_STRIP:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            break;
        case GPU_PRIM_TOPO_PATCH_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
        case GPU_PRIM_TOPO_TRI_LIST:
            vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            break;
        default:
            assert(false);
            break;
    }
    return vk_topology;
}

VkCompareOp VulkanUtil_CompareOpToVk(EGPUCompareMode compareMode)
{
    switch (compareMode)
    {
        case GPU_CMP_NEVER:
            return VK_COMPARE_OP_NEVER;
        case GPU_CMP_LESS:
            return VK_COMPARE_OP_LESS;
        case GPU_CMP_EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case GPU_CMP_LEQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case GPU_CMP_GREATER:
            return VK_COMPARE_OP_GREATER;
        case GPU_CMP_NOTEQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case GPU_CMP_GEQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case GPU_CMP_ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
    }
    return VK_COMPARE_OP_NEVER;
}

VkStencilOp VulkanUtil_StencilOpToVk(EGPUStencilOp op)
{
    switch (op)
    {
        case GPU_STENCIL_OP_KEEP:
            return VK_STENCIL_OP_KEEP;
        case GPU_STENCIL_OP_SET_ZERO:
            return VK_STENCIL_OP_ZERO;
        case GPU_STENCIL_OP_REPLACE:
            return VK_STENCIL_OP_REPLACE;
        case GPU_STENCIL_OP_INVERT:
            return VK_STENCIL_OP_INVERT;
        case GPU_STENCIL_OP_INCR:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case GPU_STENCIL_OP_DECR:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        case GPU_STENCIL_OP_INCR_SAT:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case GPU_STENCIL_OP_DECR_SAT:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    }
    return VK_STENCIL_OP_KEEP;
}


// Shader Reflection
static const EGPUResourceType RTLut[] = {
    GPU_RESOURCE_TYPE_SAMPLER,                // SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER
    GPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER, // SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    GPU_RESOURCE_TYPE_TEXTURE,                // SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    GPU_RESOURCE_TYPE_RW_TEXTURE,             // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE
    GPU_RESOURCE_TYPE_TEXEL_BUFFER,           // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
    GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER,        // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
    GPU_RESOURCE_TYPE_UNIFORM_BUFFER,         // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER
    GPU_RESOURCE_TYPE_RW_BUFFER,              // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER
    GPU_RESOURCE_TYPE_UNIFORM_BUFFER,         // SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
    GPU_RESOURCE_TYPE_RW_BUFFER,              // SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
    GPU_RESOURCE_TYPE_INPUT_ATTACHMENT,       // SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
    GPU_RESOURCE_TYPE_RAY_TRACING             // SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
};

static EGPUTextureDimension DIMLut[SpvDimSubpassData + 1] = {
    GPU_TEX_DIMENSION_1D,        // SpvDim1D
    GPU_TEX_DIMENSION_2D,        // SpvDim2D
    GPU_TEX_DIMENSION_3D,        // SpvDim3D
    GPU_TEX_DIMENSION_CUBE,      // SpvDimCube
    GPU_TEX_DIMENSION_UNDEFINED, // SpvDimRect
    GPU_TEX_DIMENSION_UNDEFINED, // SpvDimBuffer
    GPU_TEX_DIMENSION_UNDEFINED  // SpvDimSubpassData
};
static EGPUTextureDimension ArrDIMLut[SpvDimSubpassData + 1] = {
    GPU_TEX_DIMENSION_1D_ARRAY,   // SpvDim1D
    GPU_TEX_DIMENSION_2D_ARRAY,   // SpvDim2D
    GPU_TEX_DIMENSION_UNDEFINED,  // SpvDim3D
    GPU_TEX_DIMENSION_CUBE_ARRAY, // SpvDimCube
    GPU_TEX_DIMENSION_UNDEFINED,  // SpvDimRect
    GPU_TEX_DIMENSION_UNDEFINED,  // SpvDimBuffer
    GPU_TEX_DIMENSION_UNDEFINED   // SpvDimSubpassData
};

const char8_t* push_constants_name = u8"push_constants";
void VulkanUtil_InitializeShaderReflection(GPUDeviceID device, GPUShaderLibrary_Vulkan* S, const struct GPUShaderLibraryDescriptor* desc)
{
    S->pReflect             = (SpvReflectShaderModule*)malloc(sizeof(SpvReflectShaderModule));
    SpvReflectResult spvRes = spvReflectCreateShaderModule(desc->codeSize, desc->code, S->pReflect);
    (void)spvRes;
    assert(spvRes == SPV_REFLECT_RESULT_SUCCESS && "Failed to Reflect Shader!");
    uint32_t entry_count       = S->pReflect->entry_point_count;
    S->super.entrys_count      = entry_count;
    S->super.entry_reflections = (GPUShaderReflection*)malloc(entry_count * sizeof(GPUShaderReflection));
    memset(S->super.entry_reflections, 0, entry_count * sizeof(GPUShaderReflection));
    for (uint32_t i = 0; i < entry_count; i++)
    {
        // Initialize Common Reflection Data
        GPUShaderReflection* reflection = &S->super.entry_reflections[i];
        // ATTENTION: We have only one entry point now
        const SpvReflectEntryPoint* entry = spvReflectGetEntryPoint(S->pReflect, S->pReflect->entry_points[i].name);
        reflection->entry_name            = (const char8_t*)entry->name;
        reflection->stage                 = (EGPUShaderStage)entry->shader_stage;
        if (reflection->stage == GPU_SHADER_STAGE_COMPUTE)
        {
            reflection->thread_group_sizes[0] = entry->local_size.x;
            reflection->thread_group_sizes[1] = entry->local_size.y;
            reflection->thread_group_sizes[2] = entry->local_size.z;
        }
        const bool bGLSL = S->pReflect->source_language & SpvSourceLanguageGLSL;
        (void)bGLSL;
        const bool bHLSL = S->pReflect->source_language & SpvSourceLanguageHLSL;
        uint32_t icount;
        spvReflectEnumerateInputVariables(S->pReflect, &icount, NULL);
        if (icount > 0)
        {
            DECLEAR_ZERO_VAL(SpvReflectInterfaceVariable*, input_vars, icount)
            spvReflectEnumerateInputVariables(S->pReflect, &icount, input_vars);
            if ((entry->shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT))
            {
                reflection->vertex_inputs_count = icount;
                reflection->vertex_inputs       = (GPUVertexInput*)calloc(icount, sizeof(GPUVertexInput));
                // Handle Vertex Inputs
                for (uint32_t i = 0; i < icount; i++)
                {
                    // We use semantic for HLSL sources because DXC is a piece of shit.
                    reflection->vertex_inputs[i].name =
                    bHLSL ? (char8_t*)input_vars[i]->semantic : (char8_t*)input_vars[i]->name;
                    reflection->vertex_inputs[i].format =
                    GPUVulkanFormatToGPUFormat((VkFormat)input_vars[i]->format);
                }
            }
        }
        // Handle Descriptor Sets
        uint32_t scount;
        uint32_t ccount;
        spvReflectEnumeratePushConstantBlocks(S->pReflect, &ccount, NULL);
        spvReflectEnumerateDescriptorSets(S->pReflect, &scount, NULL);
        if (scount > 0 || ccount > 0)
        {
            DECLEAR_ZERO_VAL(SpvReflectDescriptorSet*, descriptros_sets, scount + 1)
            DECLEAR_ZERO_VAL(SpvReflectBlockVariable*, root_sets, ccount + 1)
            spvReflectEnumerateDescriptorSets(S->pReflect, &scount, descriptros_sets);
            spvReflectEnumeratePushConstantBlocks(S->pReflect, &ccount, root_sets);
            uint32_t bcount = 0;
            for (uint32_t i = 0; i < scount; i++)
            {
                bcount += descriptros_sets[i]->binding_count;
            }
            bcount += ccount;
            reflection->shader_resources_count = bcount;
            reflection->shader_resources       = (CGPUShaderResource*)calloc(bcount, sizeof(GPUShaderResource));
            // Fill Shader Resources
            uint32_t i_res = 0;
            for (uint32_t i_set = 0; i_set < scount; i_set++)
            {
                SpvReflectDescriptorSet* current_set = descriptros_sets[i_set];
                for (uint32_t i_binding = 0; i_binding < current_set->binding_count; i_binding++, i_res++)
                {
                    SpvReflectDescriptorBinding* current_binding = current_set->bindings[i_binding];
                    CGPUShaderResource* current_res              = &reflection->shader_resources[i_res];
                    current_res->set                             = current_binding->set;
                    current_res->binding                         = current_binding->binding;
                    current_res->stages                          = S->pReflect->shader_stage;
                    current_res->type                            = RTLut[current_binding->descriptor_type];
                    current_res->name                            = (char8_t*)current_binding->name;
                   /* current_res->name_hash =
                    cgpu_name_hash(current_binding->name, strlen(current_binding->name));*/
                    current_res->size = current_binding->count;
                    // Solve Dimension
                    if ((current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE) ||
                        (current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE))
                    {
                        if (current_binding->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY)
                            current_res->dim = ArrDIMLut[current_binding->image.dim];
                        else
                            current_res->dim = DIMLut[current_binding->image.dim];
                        if (current_binding->image.ms)
                        {
                            current_res->dim = current_res->dim & GPU_TEX_DIMENSION_2D ? GPU_TEX_DIMENSION_2DMS : current_res->dim;
                            current_res->dim = current_res->dim & GPU_TEX_DIMENSION_2D_ARRAY ? GPU_TEX_DIMENSION_2DMS_ARRAY : current_res->dim;
                        }
                    }
                }
            }
            // Fill Push Constants
            for (uint32_t i = 0; i < ccount; i++)
            {
                CGPUShaderResource* current_res = &reflection->shader_resources[i_res + i];
                current_res->set                = 0;
                current_res->type               = GPU_RESOURCE_TYPE_PUSH_CONSTANT;
                current_res->binding            = 0;
                current_res->name               = push_constants_name;
                /*current_res->name_hash =
                cgpu_name_hash(current_res->name, strlen(current_res->name));*/
                current_res->stages = S->pReflect->shader_stage;
                current_res->size   = root_sets[i]->size;
                current_res->offset = root_sets[i]->offset;
            }
        }
    }
}

void VulkanUtil_FreeShaderReflection(GPUShaderLibrary_Vulkan* S)
{
    spvReflectDestroyShaderModule(S->pReflect);
    if (S->super.entry_reflections)
    {
        for (uint32_t i = 0; i < S->super.entrys_count; i++)
        {
            GPUShaderReflection* reflection = S->super.entry_reflections + i;
            if (reflection->vertex_inputs) free(reflection->vertex_inputs);
            if (reflection->shader_resources) free(reflection->shader_resources);
        }
    }
    GPU_SAFE_FREE(S->super.entry_reflections);
    GPU_SAFE_FREE(S->pReflect);
}