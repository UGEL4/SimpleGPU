#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include <iostream>

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
    }
}

void RenderGraphBackend::Initialize()
{
    RenderGraph::Initialize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FILGHT; i++)
    {
        mExecuters[i].Initialize(m_pDevice, m_pQueue);
    }
}

void RenderGraphBackend::Finalize()
{
    RenderGraph::Finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FILGHT; i++)
    {
        mExecuters[i].Finalize();
    }
}
//////////////////RenderGraphBackend////////////////////////