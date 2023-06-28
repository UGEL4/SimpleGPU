#include "render_graph/include/frontend/RenderGraph.h"
#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include <iostream>

RenderGraph* RenderGraph::Create(const RenderGraphSetupFunc& setup)
{
    RenderGraph* graph = nullptr;

    setup();

    graph = new RenderGraphBackend();
    graph->Initialize();

    return graph;
}

void RenderGraph::Compile()
{
    std::cout << "RenderGraph::Compile()" << std::endl;
}

void RenderGraph::Execute()
{
    std::cout << "RenderGraph::Execute" << std::endl;
}

void RenderGraph::Initialize()
{
    m_pGraph = DependencyGraph::Create();
}

void RenderGraph::Finalize()
{
    DependencyGraph::Destroy(m_pGraph);
}

PassHandle RenderGraph::AddRenderPass(const RenderPassSetupFunc& setup)
{
    PassNode* newPass = new RenderPassNode();
    mPasses.emplace_back(newPass);
    m_pGraph->Insert(newPass);
    setup(*this);
    return newPass->GetHandle();
}

RenderGraph::RenderGraph()
{

}

void RenderGraph::Destroy(RenderGraph* graph)
{
    if (graph)
    {
        graph->Finalize();
        delete graph;
    }
}