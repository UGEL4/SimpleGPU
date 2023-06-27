#include "render_graph/include/frontend/RenderGraph.h"
#include "render_graph/include/backend/RenderGraphBackend.h"
#include <iostream>

RenderGraph* RenderGraph::Create(const RenderGraphSetupFunc& setup)
{
    RenderGraph* graph = nullptr;

    setup();

    graph = new RenderGraphBackend();
    graph->Initialize();

    return graph;
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