#pragma once
#include "api.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus extern "C" {

    const GPUProcTable* GPUD3D12ProcTable();

    static const D3D_FEATURE_LEVEL d3d_feature_levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0
    };

    //instance api
	GPUInstanceID CreateInstance_D3D12(const GPUInstanceDescriptor* pDesc);
	void FreeInstance_D3D12(GPUInstanceID pInstance);

    void GPUEnumerateAdapters_D3D12(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount);

    GPUDeviceID GPUCreateDevice_D3D12(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc);

    typedef struct GPUInstance_D3D12
    {
        GPUInstance super;
        struct IDXGIFactory6* pIDXGIFactory;
        ID3D12Debug* pDebugController;

        //physica device
		struct GPUAdapter_D3D12* pAdapters;
        uint32_t adaptersCount;
    } GPUInstance_D3D12;

    typedef struct GPUAdapter_D3D12
    {
        GPUAdapter super;
        IDXGIAdapter4* pIDXAdapter;
        D3D_FEATURE_LEVEL featureLevel;
    } GPUAdapter_D3D12;

    typedef struct GPUDevice_D3D12
    {
        GPUDevice super;
        ID3D12Device* pDevice;
    } GPUDevice_D3D12;

#ifdef __cplusplus
}
#endif // __cplusplus extern "C" }