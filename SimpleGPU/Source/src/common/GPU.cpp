#include "api.h"

#ifdef GPU_USE_VULKAN
    #include "backend/vulkan/GPUVulkan.h"
#endif

#include <assert.h>
#include <cstring>

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

GPUShaderLibraryID GPUCreateShaderLibrary(GPUDeviceID pDevice, GPUShaderLibraryDescriptor* pDesc)
{
    assert(pDevice);
    assert(pDevice->pProcTableCache->CreateShaderLibrary);
    GPUShaderLibrary* pShader = (GPUShaderLibrary*)pDevice->pProcTableCache->CreateShaderLibrary(pDevice, pDesc);
    pShader->pDevice          = pDevice;
    return pShader;
}

void GPUFreeShaderLibrary(GPUShaderLibraryID pShader)
{
    assert(pShader);
    assert(pShader->pDevice);
    assert(pShader->pDevice->pProcTableCache->FreeShaderLibrary);
    pShader->pDevice->pProcTableCache->FreeShaderLibrary(pShader);
}
