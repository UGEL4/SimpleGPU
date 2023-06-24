#include "backend/d3d12/GPUD3D12.h"
#include <combaseapi.h>
#include <corecrt_malloc.h>
#include <d3d12.h>
#include <stdlib.h>
#include <memory>
#include <winerror.h>
#include <assert.h>
#include <winnt.h>
#include "backend/d3d12/GPUD3D12Utils.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

GPUInstanceID CreateInstance_D3D12(const GPUInstanceDescriptor* pDesc)
{
    //void* ptr            = calloc(1, sizeof(GPUInstance_D3D12));
    void* ptr            = _aligned_malloc(sizeof(GPUInstance_D3D12), _alignof(sizeof(GPUInstance_D3D12)));
    memset(ptr, 0, sizeof(GPUInstance_D3D12));
    GPUInstance_D3D12* I = new (ptr) GPUInstance_D3D12();

    //debug layses
    D3D12Util_EnableDebugLayer(I, pDesc);

    if (SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&I->pIDXGIFactory))))
    {
        //query all adapters
        uint32_t count = 0;
        D3D12Util_QueryAllAdapters(I, &count);
        if (!count)
        {
            //no gpu physical device can use;
            assert(0);
        }
    }
    else
    {
        assert(0);
    }

    return &I->super;
}

void FreeInstance_D3D12(GPUInstanceID pInstance)
{
    GPUInstance_D3D12* I = (GPUInstance_D3D12*)pInstance;
    assert(I);

    for (uint32_t i = 0; i < I->adaptersCount; i++)
    {
        if (I->pAdapters[i].pIDXAdapter) I->pAdapters[i].pIDXAdapter->Release();
    }
    free(I->pAdapters);

    if (I->pIDXGIFactory) I->pIDXGIFactory->Release();

    if (I->pDebugController) I->pDebugController->Release();

    I->~GPUInstance_D3D12();
    _aligned_free(I);
}

void GPUEnumerateAdapters_D3D12(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount)
{
    GPUInstance_D3D12* I = (GPUInstance_D3D12*)pInstance;
    *adapterCount = I->adaptersCount;
    if (!ppAdapters) return;
    for (uint32_t i = 0; i < I->adaptersCount; i++)
    {
        ppAdapters[i] = &I->pAdapters[i].super;
    }
}

GPUDeviceID GPUCreateDevice_D3D12(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc)
{
    GPUAdapter_D3D12* A = (GPUAdapter_D3D12*)pAdapter;
    GPUDevice_D3D12* D = (GPUDevice_D3D12*)calloc(1, sizeof(GPUDevice_D3D12));

    HRESULT result = D3D12CreateDevice(A->pIDXAdapter, A->featureLevel, IID_PPV_ARGS(&D->pDevice));
    if (FAILED(result))
    {
        assert(0);
    }
    *const_cast<GPUAdapterID*>(&D->super.pAdapter) = pAdapter;
    return &D->super;
}