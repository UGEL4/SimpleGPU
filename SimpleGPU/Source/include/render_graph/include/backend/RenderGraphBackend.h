#pragma once

#include "render_graph/include/frontend/RenderGraph.h"
#include "api.h"

#define RG_MAX_FRAME_IN_FILGHT 3

class RenderPassNode;
class RenderGraphFrameExecutor
{
public:
    friend class RenderGraphBackend;
    RenderGraphFrameExecutor() = default;

    void Initialize(GPUDeviceID gfxDevice, GPUQueueID gfxQueue);
    void Finalize();
private:
    GPUCommandPoolID m_pCommandPool = nullptr;
    GPUCommandBufferID m_pCmd       = nullptr;
    GPUFenceID m_pFence             = nullptr;
};

class RenderGraphBackend : public RenderGraph
{
public:
    RenderGraphBackend(const RenderGraphBuilder& builder);
    ~RenderGraphBackend() = default;

    virtual void Execute() override;
    virtual void Initialize() override;
    virtual void Finalize() override;

    void ExecuteRenderPass(RenderPassNode* pass, RenderGraphFrameExecutor& executor);

private:
    void CalculateResourceBarriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        std::vector<GPUTextureBarrier>& tex_barriers, std::vector<std::pair<TextureHandle, GPUTextureID>>& resolved_textures);

private:
    GPUDeviceID m_pDevice;
    GPUQueueID m_pQueue;
    RenderGraphFrameExecutor mExecutors[RG_MAX_FRAME_IN_FILGHT];
};