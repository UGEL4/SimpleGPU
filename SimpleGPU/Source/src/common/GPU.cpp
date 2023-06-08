#include "api.h"

#ifdef GPU_USE_VULKAN
    #include "backend/vulkan/GPUVulkan.h"
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

    return pSwapchain;
}

void GPUFreeSwapchain(GPUSwapchainID pSwapchain)
{
    assert(pSwapchain);
    GPUDeviceID pDevice = pSwapchain->pDevice;
    assert(pDevice->pProcTableCache->FreeSwapchain);
    pDevice->pProcTableCache->FreeSwapchain(pSwapchain);
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
