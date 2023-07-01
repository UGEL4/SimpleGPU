#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include <iostream>
#include <stdint.h>

//////////////////RenderGraphFrameExecutor////////////////////////
void RenderGraphFrameExecutor::Initialize(GPUDeviceID gfxDevice, GPUQueueID gfxQueue)
{
    m_pCommandPool                  = GPUCreateCommandPool(gfxQueue);
    GPUCommandBufferDescriptor desc = {};
    desc.isSecondary                = false;
    m_pCmd                          = GPUCreateCommandBuffer(m_pCommandPool, &desc);
    m_pFence                        = GPUCreateFence(gfxDevice);
}

void RenderGraphFrameExecutor::Finalize()
{
    if (m_pCmd) GPUFreeCommandBuffer(m_pCmd);
    if (m_pCommandPool) GPUFreeCommandPool(m_pCommandPool);
    if (m_pFence) GPUFreeFence(m_pFence);
    m_pCommandPool = nullptr;
    m_pCmd         = nullptr;
    m_pFence       = nullptr;
}
//////////////////RenderGraphFrameExecutor////////////////////////

//////////////////RenderGraphBackend////////////////////////
RenderGraphBackend::RenderGraphBackend(const RenderGraphBuilder& builder)
: m_pDevice(builder.m_pDevice), m_pQueue(builder.m_pQueue)
{

}

void RenderGraphBackend::Execute()
{
    std::cout << "RenderGraphBackend::Execute" << std::endl;

    uint32_t frameIndex = mFrameIndex % RG_MAX_FRAME_IN_FILGHT;
    auto& executor = mExecutors[frameIndex];
    GPUWaitFences(&executor.m_pFence, 1);

    GPUCmdBegin(executor.m_pCmd);
    {
        for (auto& pass : mPasses)
        {
            if (pass->type == EObjectType::Pass)
            {
                ExecuteRenderPass(static_cast<RenderPassNode*>(pass), executor);
            }
        }
    }
    GPUCmdEnd(executor.m_pCmd);

    //clear
    {
        //pass
        for (auto pass : mPasses)
        {
            if (pass)
            {
                pass->ForEachTextures([this](TextureNode* texture, TextureEdge* edge)
                {
                    if (edge) delete edge;
                });
                delete pass;
            }
        }
        mPasses.clear();

        //resources
        for (auto res : mResources)
        {
            if (res)
            {
                delete res;
            }
        }
        mResources.clear();

        m_pGraph->Clear();
    }
}

void RenderGraphBackend::Initialize()
{
    RenderGraph::Initialize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FILGHT; i++)
    {
        mExecutors[i].Initialize(m_pDevice, m_pQueue);
    }
}

void RenderGraphBackend::Finalize()
{
    RenderGraph::Finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FILGHT; i++)
    {
        mExecutors[i].Finalize();
    }
}

void RenderGraphBackend::ExecuteRenderPass(RenderPassNode* pass,  RenderGraphFrameExecutor& executor)
{
    //// resource de-virtualize
    //alloca & update descriptorset
    //call gpu aip
    //deallace resource
}

void RenderGraphBackend::CalculateResourceBarriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        std::vector<GPUTextureBarrier>& tex_barriers, std::vector<std::pair<TextureHandle, GPUTextureID>>& resolved_textures)
{
    tex_barriers.resize(pass->GetTextureCount());
    resolved_textures.resize(pass->GetTextureCount());
    //遍历pass的每一个texture资源
    pass->ForEachTextures([&](TextureNode* tex, TextureEdge* edge)
    {
        //分配texture资源

        if (curr_state == edge->mRequestedState) return;
        //分配barrier
        GPUTextureBarrier barrier{};
        barrier.texture = texture;
        barrier.src_state = curr_state;
        barrier.dst_state = edge->mRequestedState;
        tex_barriers.emplace_back(barrier);
    });

    //遍历pass的每一个buffer资源
}
//////////////////RenderGraphBackend////////////////////////