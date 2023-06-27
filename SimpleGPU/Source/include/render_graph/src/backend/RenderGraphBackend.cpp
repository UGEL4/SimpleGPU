#include "render_graph/include/backend/RenderGraphBackend.h"
#include <iostream>

RenderGraphBackend::RenderGraphBackend()
{

}

void RenderGraphBackend::Execute()
{
    std::cout << "RenderGraphBackend::Execute" << std::endl;
}

void RenderGraphBackend::Initialize()
{
    RenderGraph::Initialize();
}

void RenderGraphBackend::Finalize()
{
    RenderGraph::Finalize();
}