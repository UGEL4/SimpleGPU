#include "backend/vulkan/GPUVulkan.h"

const GPUProcTable vkTable = {
    .CreateInstance                    = &CreateInstance_Vulkan,
    .FreeInstance                      = &FreeInstance_Vllkan,
    .EnumerateAdapters                 = &EnumerateAdapters_Vulkan,
    .CreateDevice                      = &CreateDevice_Vulkan,
    .FreeDevice                        = &FreeDevice_Vulkan,
    .GetQueue                          = &GetQueue_Vulkan,
    .FreeQueue                         = &GPUFreeQueue_Vulkan,
    .SubmitQueue                       = &GPUSubmitQueue_Vulkan,
    .WaitQueueIdle                     = &GPUWaitQueueIdle_Vulkan,
    .QueuePresent                      = &GPUQueuePresent_Vulkan,
    .CreateSwapchain                   = &GPUCreateSwapchain_Vulkan,
    .FreeSwapchain                     = &GPUFreeSwapchain_Vulkan,
    .AcquireNextImage                  = &GPUAcquireNextImage_Vulkan,
    .CreateTextureView                 = &GPUCreateTextureView_Vulkan,
    .FreeTextureView                   = &GPUFreeTextureView_Vulkan,
    .CreateTexture                     = &GPUCreateTexture_Vulkan,
    .FreeTexture                       = &GPUFreeTexture_Vulkan,
    .CreateShaderLibrary               = &GPUCreateShaderLibrary_Vulkan,
    .FreeShaderLibrary                 = &GPUFreeShaderLibrary_Vulkan,
    .CreateRenderPipeline              = &GPUCreateRenderPipeline_Vulkan,
    .FreeRenderPipeline                = &GPUFreeRenderPipeline_Vulkan,
    .CreateRootSignature               = &GPUCreateRootSignature_Vulkan,
    .FreeRootSignature                 = &GPUFreeRootSignature_Vulkan,
    .CreateCommandPool                 = &GPUCreateCommandPool_Vulkan,
    .FreeCommandPool                   = &GPUFreeCommandPool_Vulkan,
    .ResetCommandPool                  = &GPUResetCommandPool_Vulkan,
    .CreateCommandBuffer               = &GPUCreateCommandBuffer_Vulkan,
    .FreeCommandBuffer                 = &GPUFreeCommandBuffer_Vulkan,
    .CmdBegin                          = &GPUCmdBegin_Vulkan,
    .CmdEnd                            = &GPUCmdEnd_Vulkan,
    .CmdResourceBarrier                = &GPUCmdResourceBarrier_Vulkan,
    .CmdTransferBufferToTexture        = &GPUCmdTransferBufferToTexture_Vulkan,
    .CreateFence                       = &GPUCreateFence_Vulkan,
    .FreeFence                         = &GPUFreeFence_Vulkan,
    .WaitFences                        = &GPUWaitFences_Vulkan,
    .QueryFenceStatus                  = &GPUQueryFenceStatus_Vulkan,
    .GpuCreateSemaphore                = &GPUCreateSemaphore_Vulkan,
    .GpuFreeSemaphore                  = &GPUFreeSemaphore_Vulkan,
    .CmdBeginRenderPass                = &GPUCmdBeginRenderPass_Vulkan,
    .CmdEndRenderPass                  = &GPUCmdEndRenderPass_Vulkan,
    .RenderEncoderSetViewport          = &GPURenderEncoderSetViewport_Vulkan,
    .RenderEncoderSetScissor           = &GPURenderEncoderSetScissor_Vulkan,
    .RenderEncoderBindPipeline         = &GPURenderEncoderBindPipeline_Vulkan,
    .RenderEncoderDraw                 = &GPURenderEncoderDraw_Vulkan,
    .RenderEncoderDrawIndexed          = &GPURenderEncoderDrawIndexed_Vulkan,
    .RenderEncoderDrawIndexedInstanced = &GPURenderEncoderDrawIndexedInstanced_Vulkan,
    .RenderEncoderBindVertexBuffers    = &GPURenderEncoderBindVertexBuffers_Vulkan,
    .RenderEncoderBindIndexBuffer      = &GPURenderEncoderBindIndexBuffer_Vulkan,
    .RenderEncoderBindDescriptorSet    = &GPURenderEncoderBindDescriptorSet_Vulkan,
    .CreateBuffer                      = &GPUCreateBuffer_Vulkan,
    .FreeBuffer                        = &GPUFreeBuffer_Vulkan,
    .TransferBufferToBuffer            = &GPUTransferBufferToBuffer_Vulkan,
    .CreateSampler                     = &GPUCreateSampler_Vulkan,
    .FreeSampler                       = &GPUFreeSampler_Vulkan,
    .CreateDescriptorSet               = &GPUCreateDescriptorSet_Vulkan,
    .FreeDescriptorSet                 = &GPUFreeDescriptorSet_Vulkan,
    .UpdateDescriptorSet               = &GPUUpdateDescriptorSet_Vulkan,
};
const GPUProcTable* GPUVulkanProcTable()
{
    return &vkTable;
}