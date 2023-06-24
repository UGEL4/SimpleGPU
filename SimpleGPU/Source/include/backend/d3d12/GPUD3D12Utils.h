#pragma once
#include "GPUD3D12.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    void D3D12Util_QueryAllAdapters(GPUInstance_D3D12* pInstance, uint32_t* adapterCount);
    void D3D12Util_EnableDebugLayer(GPUInstance_D3D12* pInstance, const GPUInstanceDescriptor* pDesc);

#ifdef __cplusplus
}
#endif // __cplusplus