#include "backend/d3d12/GPUD3D12.h"

const GPUProcTable d3d12_table = {
    .CreateInstance = &CreateInstance_D3D12,
    .FreeInstance = &FreeInstance_D3D12
};

const GPUProcTable* GPUD3D12ProcTable() { return &d3d12_table; }