#include "backend/d3d12/GPUD3D12Utils.h"
#include <combaseapi.h>
#include <corecrt_malloc.h>
#include <dxgi.h>
#include <winerror.h>
#include <vector>
#include <stdlib.h>

void D3D12Util_QueryAllAdapters(GPUInstance_D3D12* pInstance, uint32_t* adapterCount)
{
    uint32_t i = 0;
    IDXGIAdapter* adapter = nullptr;
    std::vector<IDXGIAdapter4*> adapterList;
    std::vector<D3D_FEATURE_LEVEL> adapterLevels;
    while(pInstance->pIDXGIFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        DXGI_ADAPTER_DESC3 desc{};
        IDXGIAdapter4* _adapter = nullptr;
        adapter->QueryInterface(IID_PPV_ARGS(&_adapter));
        _adapter->GetDesc3(&desc);
        if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
        {
            uint32_t level_c = sizeof(d3d_feature_levels) / sizeof(D3D_FEATURE_LEVEL);
            for (uint32_t level = 0; level < level_c; ++level)
            {
                // Make sure the adapter can support a D3D12 device
                ID3D12Device* pDevice = nullptr;
                if (SUCCEEDED(D3D12CreateDevice(_adapter, d3d_feature_levels[level], __uuidof(ID3D12Device), (void**)&pDevice)))
                {
                    GPUAdapter_D3D12 gpuAdapter{};
                    HRESULT hres = _adapter->QueryInterface(IID_PPV_ARGS(&gpuAdapter.pIDXAdapter));
                    if (SUCCEEDED(hres))
                    {
                        pDevice->Release();
                        gpuAdapter.pIDXAdapter->Release();
                        pInstance->adaptersCount++;
                        // Add ref
                        { 
                            adapterList.push_back(_adapter);
                            adapterLevels.push_back(d3d_feature_levels[level]);
                        }
                        break;
                    }
                }
            }
        }
        if (adapter)
        {
            adapter->Release();
            adapter = nullptr;
        }

        i++;
    }
    *adapterCount = pInstance->adaptersCount;
    pInstance->pAdapters = (GPUAdapter_D3D12*)calloc(pInstance->adaptersCount, sizeof(GPUAdapter_D3D12));
    for (uint32_t i = 0; i < pInstance->adaptersCount; i++)
    {
        GPUAdapter_D3D12* adapter = &pInstance->pAdapters[i];
        adapter->pIDXAdapter      = adapterList[i];
        adapter->featureLevel     = adapterLevels[i];
        adapter->super.pInstance  = &pInstance->super;

        //TODO: record details
    }
}

void D3D12Util_EnableDebugLayer(GPUInstance_D3D12* pInstance, const GPUInstanceDescriptor* pDesc)
{
    if (pDesc->enableDebugLayer)
    {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pInstance->pDebugController))))
        {
            pInstance->pDebugController->EnableDebugLayer();
        }
    }
}