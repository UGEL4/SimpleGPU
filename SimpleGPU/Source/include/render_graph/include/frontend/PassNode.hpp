#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include <vector>
#include <functional>
#include <span>
#include <iostream>

class TextureEdge;
class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    PassHandle const GetHandle() const;
    void ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>&);
    ~PassNode() {std::cout << "Free PassNode : " << mId << std::endl;}

    uint32_t GetTextureCount() const { return (int32_t)(mOutTextureEdges.size() + mInTextureEdges.size()); }
    const bool Before(const PassNode* other) const;
    const bool After(const PassNode* other) const;
    std::span<TextureReadEdge*> GetTextureReadEdges();
    std::span<TextureWriteEdge*> GetTextureWriteEdges();
protected:
    PassNode(EPassType type, uint32_t order);

protected:
    std::vector<TextureWriteEdge*> mOutTextureEdges;
    std::vector<TextureReadEdge*> mInTextureEdges;
    uint32_t mOrder;
    const EPassType mPassType = EPassType::None;
    bool mCanBeLone = false;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    RenderPassNode(uint32_t order);

private:
    RenderPassExecuteFunction mExecuteFunc;
    GPURootSignatureID m_pRootSignature = nullptr;;
    GPURenderPipelineID m_pPipeline     = nullptr;
    EGPULoadAction mLoadActions[GPU_MAX_MRT_COUNT + 1];
    EGPUStoreAction mStoreActions[GPU_MAX_MRT_COUNT + 1];
    EGPULoadAction mDepthLoadAction;
    EGPUStoreAction mDepthStoreAction;
    EGPULoadAction mStencilLoadAction;
    EGPUStoreAction mStencilStoreAction;
    float mClearDepth;
};

class PresentPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    inline bool Reimport(GPUSwapchainID swapchain, uint32_t index)
    {
        if (!swapchain) return false;
        mDesc.swapchain = swapchain;
        mDesc.index     = index;
        return true;
    }

    PresentPassNode(uint32_t order);
private:
    GPUQueuePresentDescriptor mDesc;
};