#include "api.h"

#ifdef GPU_USE_VULKAN
    #include "backend/vulkan/GPUVulkan.h"
#endif

#ifdef GPU_USE_D3D12
    #include "backend/d3d12/GPUD3D12.h"
#endif

#include <assert.h>
#include <cstring>
#include <memory>

GPUInstanceID GPUCreateInstance(const GPUInstanceDescriptor* pDesc)
{
    assert(pDesc->backend == EGPUBackend::GPUBackend_Vulkan);

    const GPUProcTable* pProcTable             = nullptr;
    const GPUSurfacesProcTable* pSurfacesTable = nullptr;

    if (pDesc->backend == GPUBackend_Count)
    {
    }
#ifdef GPU_USE_VULKAN
    else if (pDesc->backend == GPUBackend_Vulkan)
    {
        pProcTable     = GPUVulkanProcTable();
        pSurfacesTable = GPUVulkanSurfacesTable();
    }
#endif
#ifdef GPU_USE_D3D12
    else if (pDesc->backend == GPUBackend_D3D12)
    {
        pProcTable = GPUD3D12ProcTable();
    }
#endif

    GPUInstance* pInstance       = (GPUInstance*)pProcTable->CreateInstance(pDesc);
    pInstance->pProcTable        = pProcTable;
    pInstance->pSurfaceProcTable = pSurfacesTable;
    pInstance->backend           = pDesc->backend;

    return pInstance;
}

void GPUFreeInstance(GPUInstanceID instance)
{
    assert(instance->backend == EGPUBackend::GPUBackend_Vulkan);
    assert(instance->pProcTable->FreeInstance);

    instance->pProcTable->FreeInstance(instance);
}

void GPUEnumerateAdapters(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount)
{
    assert(pInstance != NULL);
    assert(pInstance->pProcTable->EnumerateAdapters != NULL);

    pInstance->pProcTable->EnumerateAdapters(pInstance, ppAdapters, adapterCount);

    if (ppAdapters != NULL)
    {
        for (uint32_t i = 0; i < *adapterCount; i++)
        {
            *((const GPUProcTable**)&ppAdapters[i]->pProcTableCache) = pInstance->pProcTable;
            *((GPUInstanceID*)&ppAdapters[i]->pInstance)             = pInstance;
        }
    }
}

GPUDeviceID GPUCreateDevice(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc)
{
    assert(pAdapter->pProcTableCache);
    assert(pAdapter->pProcTableCache->CreateDevice);

    GPUDeviceID pDevice = pAdapter->pProcTableCache->CreateDevice(pAdapter, pDesc);
    if (pDevice != nullptr)
    {
        *(const GPUProcTable**)&pDevice->pProcTableCache = pAdapter->pProcTableCache;
    }
    ((GPUDevice*)pDevice)->nextTextureId = 0;
    return pDevice;
}

void GPUFreeDevice(GPUDeviceID pDevice)
{
    assert(pDevice->pProcTableCache);
    assert(pDevice->pProcTableCache->FreeDevice);
    pDevice->pProcTableCache->FreeDevice(pDevice);
}

GPUQueueID GPUGetQueue(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex)
{
    assert(pDevice);
    assert(pDevice->pProcTableCache);
    assert(pDevice->pProcTableCache->GetQueue);

    // try find queue
    // if (q) {warning();return q;}

    GPUQueue* pQueue   = (GPUQueue*)pDevice->pProcTableCache->GetQueue(pDevice, queueType, queueIndex);
    pQueue->pDevice    = pDevice;
    pQueue->queueType  = queueType;
    pQueue->queueIndex = queueIndex;

    return pQueue;
}

void GPUFreeQueue(GPUQueueID queue)
{
    assert(queue);
    assert(queue->pDevice);
    assert(queue->pDevice->pProcTableCache->FreeQueue);
    queue->pDevice->pProcTableCache->FreeQueue(queue);
}

void GPUSubmitQueue(GPUQueueID queue, const struct GPUQueueSubmitDescriptor* desc)
{
    assert(queue);
    assert(queue->pDevice);
    assert(queue->pDevice->pProcTableCache->SubmitQueue);
    queue->pDevice->pProcTableCache->SubmitQueue(queue, desc);
}

void GPUWaitQueueIdle(GPUQueueID queue)
{
    assert(queue);
    assert(queue->pDevice);
    assert(queue->pDevice->pProcTableCache->WaitQueueIdle);
    queue->pDevice->pProcTableCache->WaitQueueIdle(queue);
}

void GPUQueuePresent(GPUQueueID queue, const struct GPUQueuePresentDescriptor* desc)
{
    assert(queue);
    assert(queue->pDevice);
    assert(queue->pDevice->pProcTableCache->QueuePresent);
    queue->pDevice->pProcTableCache->QueuePresent(queue, desc);
}

GPUSurfaceID GPUCreateSurfaceFromNativeView(GPUInstanceID pInstance, void* view)
{
#if defined(_WIN64)
    return GPUCreateSurfaceFromHWND(pInstance, (HWND)view);
#endif

    return nullptr;
}

void GPUFreeSurface(GPUInstanceID pInstance, GPUSurfaceID pSurface)
{
    assert(pInstance);
    assert(pInstance->pSurfaceProcTable);
    assert(pInstance->pSurfaceProcTable->FreeSurface);
    pInstance->pSurfaceProcTable->FreeSurface(pInstance, pSurface);
}

#if defined(_WIN64)
GPUSurfaceID GPUCreateSurfaceFromHWND(GPUInstanceID pInstance, HWND window)
{
    assert(pInstance);
    assert(pInstance->pSurfaceProcTable);
    assert(pInstance->pSurfaceProcTable->CreateSurfaceFromHWND);
    return pInstance->pSurfaceProcTable->CreateSurfaceFromHWND(pInstance, window);
}
#endif

GPUSwapchainID GPUCreateSwapchain(GPUDeviceID pDevice, GPUSwapchainDescriptor* pDesc)
{
    assert(pDevice);
    assert(pDevice->pProcTableCache->CreateSwapchain);

    GPUSwapchain* pSwapchain = (GPUSwapchain*)pDevice->pProcTableCache->CreateSwapchain(pDevice, pDesc);
    pSwapchain->pDevice      = pDevice;
    for (uint32_t i = 0; i < pSwapchain->backBuffersCount; i++)
    {
        ((GPUTexture*)pSwapchain->ppBackBuffers[i])->uniqueId = ((GPUDevice*)pDevice)->nextTextureId++;
    }

    return pSwapchain;
}

void GPUFreeSwapchain(GPUSwapchainID pSwapchain)
{
    assert(pSwapchain);
    GPUDeviceID pDevice = pSwapchain->pDevice;
    assert(pDevice->pProcTableCache->FreeSwapchain);
    pDevice->pProcTableCache->FreeSwapchain(pSwapchain);
}

uint32_t GPUAcquireNextImage(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor* desc)
{
    assert(swapchain);
    assert(swapchain->pDevice);
    assert(swapchain->pDevice->pProcTableCache->AcquireNextImage);
    return swapchain->pDevice->pProcTableCache->AcquireNextImage(swapchain, desc);
}

GPUTextureViewID GPUCreateTextureView(GPUDeviceID pDevice, const struct GPUTextureViewDescriptor* pDesc)
{
    assert(pDevice);
    assert(pDesc);
    assert(pDevice->pProcTableCache->CreateTextureView);

    GPUTextureView* pView = (GPUTextureView*)pDevice->pProcTableCache->CreateTextureView(pDevice, pDesc);
    pView->pDevice        = pDevice;
    pView->desc           = *pDesc;

    return pView;
}

void GPUFreeTextureView(GPUTextureViewID pTextureView)
{
    assert(pTextureView);
    assert(pTextureView->pDevice);
    assert(pTextureView->pDevice->pProcTableCache->FreeTextureView);
    pTextureView->pDevice->pProcTableCache->FreeTextureView(pTextureView);
}

GPUTextureID GPUCreateTexture(GPUDeviceID device, const GPUTextureDescriptor* desc)
{
    assert(device);
    assert(device->pProcTableCache->CreateTexture);
    GPUTextureDescriptor new_desc{};
    memcpy(&new_desc, desc, sizeof(GPUTextureDescriptor));
    if (desc->array_size == 0) new_desc.array_size = 1;
    if (desc->mip_levels == 0) new_desc.mip_levels = 1;
    if (desc->depth == 0) new_desc.depth = 1;
    if (desc->sample_count == 0) new_desc.sample_count = (EGPUSampleCount)1;
    GPUTexture* T  = (GPUTexture*)device->pProcTableCache->CreateTexture(device, &new_desc);
    T->pDevice     = device;
    T->sampleCount = desc->sample_count;
    return T;
}

void GPUFreeTexture(GPUTextureID texture)
{
    assert(texture);
    assert(texture->pDevice);
    assert(texture->pDevice->pProcTableCache->FreeTexture);
    texture->pDevice->pProcTableCache->FreeTexture(texture);
}

GPUShaderLibraryID GPUCreateShaderLibrary(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc)
{
    assert(pDevice);
    assert(pDevice->pProcTableCache->CreateShaderLibrary);
    GPUShaderLibrary* pShader = (GPUShaderLibrary*)pDevice->pProcTableCache->CreateShaderLibrary(pDevice, pDesc);
    pShader->pDevice          = pDevice;
    const size_t str_len      = strlen((const char*)pDesc->pName);
    const size_t str_size     = str_len + 1;
    pShader->name             = (char8_t*)calloc(1, str_size * sizeof(char8_t));
    memcpy((void*)pShader->name, pDesc->pName, str_size);
    return pShader;
}

void GPUFreeShaderLibrary(GPUShaderLibraryID pShader)
{
    assert(pShader);
    assert(pShader->pDevice);
    assert(pShader->pDevice->pProcTableCache->FreeShaderLibrary);
    free(pShader->name);
    pShader->pDevice->pProcTableCache->FreeShaderLibrary(pShader);
}

static const GPUDepthStateDesc sDefaultDepthState = {
    .depthTest  = false,
    .depthWrite = false,
    .stencilTest = false
};

static const GPURasterizerStateDescriptor sDefaultRasterizerState = {
    .cullMode = GPU_CULL_MODE_BACK,
    .fillMode = GPU_FILL_MODE_SOLID,
    .frontFace = GPU_FRONT_FACE_CCW,
    .depthBias = 0,
    .slopeScaledDepthBias = 0.f,
    .enableMultiSample = false,
    .enableScissor = false,
    .enableDepthClamp = false
};

GPURenderPipelineID GPUCreateRenderPipeline(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc)
{
    static GPUBlendStateDescriptor sDefaulBlendState = {};
    sDefaulBlendState.srcFactors[0]           = GPU_BLEND_CONST_ONE;
    sDefaulBlendState.dstFactors[0]           = GPU_BLEND_CONST_ZERO;
    sDefaulBlendState.srcAlphaFactors[0]      = GPU_BLEND_CONST_ONE;
    sDefaulBlendState.dstAlphaFactors[0]      = GPU_BLEND_CONST_ZERO;
    sDefaulBlendState.blendModes[0]           = GPU_BLEND_MODE_ADD;
    sDefaulBlendState.masks[0]                = GPU_COLOR_MASK_ALL;
    sDefaulBlendState.independentBlend        = false;

    assert(pDevice);
    assert(pDevice->pProcTableCache->CreateRenderPipeline);
    GPURenderPipelineDescriptor newDesc{};
    memcpy(&newDesc, pDesc, sizeof(GPURenderPipelineDescriptor));
    if (pDesc->samplerCount == 0) newDesc.samplerCount = GPU_SAMPLE_COUNT_1;
    if (pDesc->pBlendState == NULL) newDesc.pBlendState = &sDefaulBlendState;
    if (pDesc->pDepthState == NULL) newDesc.pDepthState = &sDefaultDepthState;
    if (pDesc->pRasterizerState == NULL) newDesc.pRasterizerState = &sDefaultRasterizerState;
    GPURenderPipeline* pPipeline = NULL;
    pPipeline                    = (GPURenderPipeline*)pDevice->pProcTableCache->CreateRenderPipeline(pDevice, &newDesc);
    pPipeline->pDevice           = pDevice;
    pPipeline->pRootSignature    = pDesc->pRootSignature;
    return pPipeline;
}

void GPUFreeRenderPipeline(GPURenderPipelineID pPipeline)
{
    assert(pPipeline);
    assert(pPipeline->pDevice->pProcTableCache->FreeRenderPipeline);
    pPipeline->pDevice->pProcTableCache->FreeRenderPipeline(pPipeline);
}

GPURootSignatureID GPUCreateRootSignature(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc)
{
    GPURootSignature* pRST = (GPURootSignature*)device->pProcTableCache->CreateRootSignature(device, desc);
    pRST->device           = device;
    return pRST;
}

void GPUFreeRootSignature(GPURootSignatureID RS)
{
    assert(RS);
    assert(RS->device);
    assert(RS->device->pProcTableCache->FreeRootSignature);
    RS->device->pProcTableCache->FreeRootSignature(RS);
}

GPUCommandPoolID GPUCreateCommandPool(GPUQueueID queue)
{
    assert(queue);
    assert(queue->pDevice);
    assert(queue->pDevice->pProcTableCache->CreateCommandPool);
    GPUCommandPool* P = (GPUCommandPool*)queue->pDevice->pProcTableCache->CreateCommandPool(queue);
    P->queue          = queue;
    return P;
}

void GPUFreeCommandPool(GPUCommandPoolID pool)
{
    assert(pool);
    assert(pool->queue);
    assert(pool->queue->pDevice);
    assert(pool->queue->pDevice->pProcTableCache->FreeCommandPool);
    pool->queue->pDevice->pProcTableCache->FreeCommandPool(pool);
}

void GPUResetCommandPool(GPUCommandPoolID pool)
{
    assert(pool);
    assert(pool->queue);
    assert(pool->queue->pDevice);
    assert(pool->queue->pDevice->pProcTableCache->ResetCommandPool);
    pool->queue->pDevice->pProcTableCache->ResetCommandPool(pool);
}

GPUCommandBufferID GPUCreateCommandBuffer(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc)
{
    assert(pool);
    assert(pool->queue);
    assert(pool->queue->pDevice);
    assert(pool->queue->pDevice->pProcTableCache->CreateCommandBuffer);
    GPUCommandBuffer* CMD = (GPUCommandBuffer*)pool->queue->pDevice->pProcTableCache->CreateCommandBuffer(pool, desc);
    CMD->device           = pool->queue->pDevice;
    CMD->pool             = pool;
    return CMD;
}

void GPUFreeCommandBuffer(GPUCommandBufferID cmd)
{
    assert(cmd);
    assert(cmd->device);
    assert(cmd->device->pProcTableCache->FreeCommandBuffer);
    cmd->device->pProcTableCache->FreeCommandBuffer(cmd);
}

void GPUCmdBegin(GPUCommandBufferID cmdBuffer)
{
    assert(cmdBuffer);
    assert(cmdBuffer->device);
    assert(cmdBuffer->device->pProcTableCache->CmdBegin);
    cmdBuffer->device->pProcTableCache->CmdBegin(cmdBuffer);
}

void GPUCmdEnd(GPUCommandBufferID cmdBuffer)
{
    assert(cmdBuffer);
    assert(cmdBuffer->device);
    assert(cmdBuffer->device->pProcTableCache->CmdEnd);
    cmdBuffer->device->pProcTableCache->CmdEnd(cmdBuffer);
}

void GPUCmdResourceBarrier(GPUCommandBufferID cmd, const struct GPUResourceBarrierDescriptor* desc)
{
    assert(cmd);
    assert(cmd->device);
    assert(cmd->device->pProcTableCache->CmdResourceBarrier);
    cmd->device->pProcTableCache->CmdResourceBarrier(cmd, desc);
}

void GPUCmdTransferBufferToTexture(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer* desc)
{
    assert(cmd);
    assert(cmd->device);
    assert(cmd->device->pProcTableCache->CmdTransferBufferToTexture);
    cmd->device->pProcTableCache->CmdTransferBufferToTexture(cmd, desc);
}

GPUFenceID GPUCreateFence(GPUDeviceID device)
{
    assert(device);
    assert(device->pProcTableCache->CreateFence);
    GPUFence* F = (GPUFence*)device->pProcTableCache->CreateFence(device);
    F->device   = device;
    return F;
}

void GPUFreeFence(GPUFenceID fence)
{
    assert(fence);
    assert(fence->device);
    assert(fence->device->pProcTableCache->FreeFence);
    fence->device->pProcTableCache->FreeFence(fence);
}

void GPUWaitFences(const GPUFenceID* fences, uint32_t fenceCount)
{
    if (fences == NULL || fenceCount == 0) return;
    GPUFenceID fence = fences[0];
    assert(fence);
    assert(fence->device);
    assert(fence->device->pProcTableCache->WaitFences);
    fence->device->pProcTableCache->WaitFences(fences, fenceCount);
}

EGPUFenceStatus GPUQueryFenceStatus(GPUFenceID fence)
{
    assert(fence);
    assert(fence->device);
    assert(fence->device->pProcTableCache->QueryFenceStatus);
    return fence->device->pProcTableCache->QueryFenceStatus(fence);
}

GPUSemaphoreID GPUCreateSemaphore(GPUDeviceID device)
{
    assert(device);
    assert(device->pProcTableCache->GpuCreateSemaphore);
    GPUSemaphore* s = (GPUSemaphore*)device->pProcTableCache->GpuCreateSemaphore(device);
    s->device       = device;
    return s;
}

void GPUFreeSemaphore(GPUSemaphoreID semaphore)
{
    assert(semaphore);
    assert(semaphore->device);
    assert(semaphore->device->pProcTableCache->GpuFreeSemaphore);
    semaphore->device->pProcTableCache->GpuFreeSemaphore(semaphore);
}

GPURenderPassEncoderID GPUCmdBeginRenderPass(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor* desc)
{
    assert(cmd);
    assert(cmd->device);
    assert(cmd->device->pProcTableCache->CmdBeginRenderPass);
    GPURenderPassEncoderID id = cmd->device->pProcTableCache->CmdBeginRenderPass(cmd, desc);
    GPUCommandBuffer* b       = (GPUCommandBuffer*)cmd;
    b->currentDispatch        = GPU_PIPELINE_TYPE_GRAPHICS;
    return id;
}

void GPUCmdEndRenderPass(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder)
{
    assert(cmd);
    assert(cmd->device);
    assert(cmd->device->pProcTableCache->CmdEndRenderPass);
    assert(cmd->currentDispatch == GPU_PIPELINE_TYPE_GRAPHICS);
    cmd->device->pProcTableCache->CmdEndRenderPass(cmd, encoder);
    GPUCommandBuffer* b = (GPUCommandBuffer*)cmd;
    b->currentDispatch  = GPU_PIPELINE_TYPE_NONE;
}

void GPURenderEncoderSetViewport(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth)
{
    GPUDeviceID D = encoder->device;
    assert(D);
    assert(D->pProcTableCache->RenderEncoderSetViewport);
    D->pProcTableCache->RenderEncoderSetViewport(encoder, x, y, width, height, min_depth, max_depth);
}

void GPURenderEncoderSetScissor(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    GPUDeviceID D = encoder->device;
    assert(D);
    assert(D->pProcTableCache->RenderEncoderSetScissor);
    D->pProcTableCache->RenderEncoderSetScissor(encoder, x, y, width, height);
}

void GPURenderEncoderBindPipeline(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline)
{
    GPUDeviceID D = encoder->device;
    assert(D);
    assert(D->pProcTableCache->RenderEncoderBindPipeline);
    D->pProcTableCache->RenderEncoderBindPipeline(encoder, pipeline);
}

void GPURenderEncoderDraw(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex)
{
    GPUDeviceID D = encoder->device;
    assert(D);
    assert(D->pProcTableCache->RenderEncoderDraw);
    D->pProcTableCache->RenderEncoderDraw(encoder, vertex_count, first_vertex);
}

void GPURenderEncoderDrawIndexed(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
{
    GPUDeviceID D = encoder->device;
    assert(D);
    assert(D->pProcTableCache->RenderEncoderDrawIndexed);
    D->pProcTableCache->RenderEncoderDrawIndexed(encoder, indexCount, firstIndex, vertexOffset);
}

void GPURenderEncoderDrawIndexedInstanced(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    GPUDeviceID D = encoder->device;
    assert(D);
    assert(D->pProcTableCache->RenderEncoderDrawIndexedInstanced);
    D->pProcTableCache->RenderEncoderDrawIndexedInstanced(encoder, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void GPURenderEncoderBindVertexBuffers(GPURenderPassEncoderID encoder, uint32_t buffer_count,
                                       const GPUBufferID* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    assert(encoder);
    assert(encoder->device);
    assert(encoder->device->pProcTableCache->RenderEncoderBindVertexBuffers);
    assert(buffers);
    encoder->device->pProcTableCache->RenderEncoderBindVertexBuffers(encoder, buffer_count, buffers, strides, offsets);
}

void GPURenderEncoderBindIndexBuffer(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride)
{
    assert(encoder);
    assert(encoder->device);
    assert(encoder->device->pProcTableCache->RenderEncoderBindIndexBuffer);
    assert(buffer);
    encoder->device->pProcTableCache->RenderEncoderBindIndexBuffer(encoder, buffer, offset, indexStride);
}

void GPURenderEncoderBindDescriptorSet(GPURenderPassEncoderID encoder, GPUDescriptorSetID set)
{
    assert(encoder);
    assert(encoder->device);
    assert(set);
    assert(encoder->device->pProcTableCache->RenderEncoderBindDescriptorSet);
    encoder->device->pProcTableCache->RenderEncoderBindDescriptorSet(encoder, set);
}

GPUBufferID GPUCreateBuffer(GPUDeviceID device, const GPUBufferDescriptor* desc)
{
    assert(device);
    assert(device->pProcTableCache->CreateBuffer);
    GPUBufferDescriptor new_desc;
    memcpy(&new_desc, desc, sizeof(GPUBufferDescriptor));
    if (desc->flags == 0)
    {
        new_desc.flags |= GPU_BCF_NONE;
    }
    GPUBuffer* B = (GPUBuffer*)device->pProcTableCache->CreateBuffer(device, &new_desc);
    B->device    = device;
    return B;
}

void GPUFreeBuffer(GPUBufferID buffer)
{
    assert(buffer);
    assert(buffer->device);
    assert(buffer->device->pProcTableCache->FreeBuffer);
    buffer->device->pProcTableCache->FreeBuffer(buffer);
}

void GPUTransferBufferToBuffer(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc)
{
    assert(cmd);
    assert(cmd->device);
    assert(cmd->device->pProcTableCache->TransferBufferToBuffer);
    assert(desc);
    assert(desc->src);
    assert(desc->dst);
    cmd->device->pProcTableCache->TransferBufferToBuffer(cmd, desc);
}

GPUSamplerID GPUCreateSampler(GPUDeviceID device, const struct GPUSamplerDescriptor* desc)
{
    assert(device);
    assert(desc);
    assert(device->pProcTableCache->CreateSampler);
    GPUSampler* sampler = (GPUSampler*)device->pProcTableCache->CreateSampler(device, desc);
    sampler->device     = device;
    return sampler;
}

void GPUFreeSampler(GPUSamplerID sampler)
{
    assert(sampler);
    assert(sampler->device);
    assert(sampler->device->pProcTableCache->FreeSampler);
    sampler->device->pProcTableCache->FreeSampler(sampler);
}

GPUDescriptorSetID GPUCreateDescriptorSet(GPUDeviceID device, const struct GPUDescriptorSetDescriptor* desc)
{
    assert(device);
    assert(device->pProcTableCache->CreateDescriptorSet);
    GPUDescriptorSet* set = (GPUDescriptorSet*)device->pProcTableCache->CreateDescriptorSet(device, desc);
    set->root_signature   = desc->root_signature;
    set->index            = desc->set_index;
    return set;
}
void GPUFreeDescriptorSet(GPUDescriptorSetID set)
{
    assert(set);
    assert(set->root_signature);
    assert(set->root_signature->device);
    assert(set->root_signature->device->pProcTableCache->FreeDescriptorSet);
    set->root_signature->device->pProcTableCache->FreeDescriptorSet(set);
}
void GPUUpdateDescriptorSet(GPUDescriptorSetID set, const GPUDescriptorData* datas, uint32_t count)
{
    assert(set);
    assert(set->root_signature);
    assert(set->root_signature->device);
    assert(set->root_signature->device->pProcTableCache->UpdateDescriptorSet);
    assert(datas);
    set->root_signature->device->pProcTableCache->UpdateDescriptorSet(set, datas, count);
}