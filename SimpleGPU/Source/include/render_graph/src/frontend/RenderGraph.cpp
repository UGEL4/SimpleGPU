#include "render_graph/include/frontend/RenderGraph.h"
#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include <iostream>

///////////RenderGraphBuilder//////////////////
RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::WithDevice(GPUDeviceID device)
{
    m_pDevice = device;
    return *this;
}

RenderGraph::RenderGraphBuilder& RenderGraph::RenderGraphBuilder::WithGFXQueue(GPUQueueID queue)
{
    m_pQueue = queue;
    return *this;
}
///////////RenderGraphBuilder//////////////////

RenderGraph* RenderGraph::Create(const RenderGraphSetupFunc& setup)
{
    RenderGraph* graph = nullptr;

    RenderGraphBuilder builder = {};
    setup(builder);

    graph = new RenderGraphBackend(builder);
    graph->Initialize();

    return graph;
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
    m_pGraph->Clear();
}

void RenderGraph::Initialize()
{
    m_pGraph = DependencyGraph::Create();
}

void RenderGraph::Finalize()
{
    DependencyGraph::Destroy(m_pGraph);
}

///////////RenderPassBuilder//////////////////
RenderGraph::RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPassNode& node)
: mGraph(graph), mPassNode(node)
{

}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Write(TextureRTVHandle handle)
{
    //链接pass_node 和 resource_node
    TextureWriteEdge* edge = new TextureWriteEdge(handle);
    mPassNode.mOutTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(handle.mThis), edge);
    return *this;
}

PassHandle RenderGraph::AddRenderPass(const RenderPassSetupFunc& setup)
{
    RenderPassNode* newPass = new RenderPassNode();
    mPasses.emplace_back(newPass);
    m_pGraph->Insert(newPass);

    RenderPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    return newPass->GetHandle();
}
///////////RenderPassBuilder//////////////////

RenderGraph::RenderGraph()
{

}
