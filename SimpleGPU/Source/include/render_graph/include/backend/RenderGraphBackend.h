#pragma once

#include "render_graph/include/frontend/RenderGraph.h"
#include "api.h"
#include "GPUBindTable.hpp"
#include "render_graph/include/backend/TexturePool.hpp"
#include "render_graph/include/backend/TextureViewPool.hpp"
#include "render_graph/include/backend/BindTablePool.hpp"
#include <unordered_map>

#define RG_MAX_FRAME_IN_FILGHT 3

class RenderPassNode;
class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;
    RenderGraphFrameExecutor() = default;

    void Initialize(GPUDeviceID gfxDevice, GPUQueueID gfxQueue);
    void Finalize();
    void ResetOnStart();
private:
    GPUCommandPoolID m_pCommandPool = nullptr;
    GPUCommandBufferID m_pCmd       = nullptr;
    GPUFenceID m_pFence             = nullptr;
    std::unordered_map<GPURootSignatureID, BindTablePool*> mBindTablePools;
};

class RenderGraphBackend : public RenderGraph
{
public:
    RenderGraphBackend(const RenderGraphBuilder& builder);
    ~RenderGraphBackend() = default;

    virtual uint64_t Execute() override;
    virtual void Initialize() override;
    virtual void Finalize() override;

    void ExecuteRenderPass(RenderPassNode* pass, RenderGraphFrameExecutor& executor);

private:
    void CalculateResourceBarriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        std::vector<GPUTextureBarrier>& tex_barriers, std::vector<std::pair<TextureHandle, GPUTextureID>>& resolved_textures);
    GPUTextureID Resolve(RenderGraphFrameExecutor& executor, const TextureNode& texture);
    GPUBindTableID AllocateAndUpdatePassBindTable(RenderGraphFrameExecutor& executor, PassNode* pass, GPURootSignatureID root_sig);
    const GPUShaderResource* FindShaderResource(uint64_t nameHash, GPURootSignatureID rs, EGPUResourceType* type = nullptr) const;
    void DeallocaResources(PassNode* pass);

private:
    GPUDeviceID m_pDevice;
    GPUQueueID m_pQueue;
    RenderGraphFrameExecutor mExecutors[RG_MAX_FRAME_IN_FILGHT];
    RG::TexturePool mTexturePool;
    RG::TextureViewPool mTextureViewPool;
};