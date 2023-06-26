#include "render_graph/include/frontend/RenderGraph.h"
#include "render_graph/include/backend/RenderGraphBackend.h"
#include <iostream>

RenderGraph* RenderGraph::Create(RenderGraphSetupFunc &setup)
{
    RenderGraph* graph = nullptr;

    setup();

    graph = new RenderGraphBackend();

    return graph;
}

void RenderGraph::Destroy(RenderGraph* graph)
{
    if (graph)
    {
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