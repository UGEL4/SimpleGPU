#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include <iostream>

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
    }
}

void RenderGraphBackend::Initialize()
{
    RenderGraph::Initialize();
}

void RenderGraphBackend::Finalize()
{
    RenderGraph::Finalize();
}