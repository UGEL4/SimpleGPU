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

EGPUResourceState RenderGraph::GetLastestState(const TextureNode* texture, const PassNode* pending_pass)
{
    if (mPasses[0] == pending_pass) return texture->mInitState;

    PassNode* pass_iter = nullptr;
    EGPUResourceState result = texture->mInitState;

    //each write pass
    ForeachWriterPass(texture->GetHandle(),
    [&](TextureNode* tex, PassNode* pass, RenderGraphEdge* edge)
    {
        if (edge->type == ERelationshipType::TextureWrite)
        {
            auto writeEdge = static_cast<TextureWriteEdge*>(edge);
            if (pass->After(pass_iter) && pass->Before(pending_pass))
            {
                pass_iter = pass;
                result    = writeEdge->mRequestedState;
            }
        }
    });

    //each read pass
    ForeachReaderPass(texture->GetHandle(), [&](TextureNode* tex, PassNode* pass, RenderGraphEdge* edge)
    {
        if (edge->type == ERelationshipType::TextureRead)
        {
            auto readEdge = static_cast<TextureReadEdge*>(edge);
            if (pass->After(pass_iter) && pass->Before(pending_pass))
            {
                pass_iter = pass;
                result    = readEdge->mRequestedState;
            }
        }
    });

    return result;
}

void RenderGraph::ForeachWriterPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>& func)
{
    m_pGraph->ForeachIncomingEdges(handle, [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) 
    {
        PassNode* node            = static_cast<PassNode*>(from);
        TextureNode* tex          = static_cast<TextureNode*>(to);
        RenderGraphEdge* tex_edge = static_cast<RenderGraphEdge*>(e);
        func(tex, node, tex_edge);
    });
}

void RenderGraph::ForeachReaderPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>& func)
{
    m_pGraph->ForeachOutgoingEdges(handle, [&](DependencyGraphNode* from, DependencyGraphNode* to, DependencyGraphEdge* e) 
    {
        PassNode* node            = static_cast<PassNode*>(to);
        TextureNode* tex          = static_cast<TextureNode*>(from);
        RenderGraphEdge* tex_edge = static_cast<RenderGraphEdge*>(e);
        func(tex, node, tex_edge);
    });
}

///////////RenderPassBuilder//////////////////
RenderGraph::RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPassNode& node)
: mGraph(graph), mPassNode(node)
{

}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetName(const char* name)
{
    if (name) mPassNode.SetName(name);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Write(TextureRTVHandle handle)
{
    //链接pass_node 和 resource_node
    TextureWriteEdge* edge = new TextureWriteEdge(handle);
    mPassNode.mOutTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(handle.mThis), edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Read(const char* name, TextureSRVHandle handle)
{
    TextureReadEdge* edge = new TextureReadEdge(name, handle);
    mPassNode.mInTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(handle.mThis), &mPassNode, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetRootSignature(GPURootSignatureID rs)
{
    mPassNode.m_pRootSignature = rs;
    return *this;
}

PassHandle RenderGraph::AddRenderPass(const RenderPassSetupFunc& setup, const RenderPassExecuteFunction& execute)
{
    uint32_t order = (uint32_t)mPasses.size();
    RenderPassNode* newPass = new RenderPassNode(order);
    mPasses.emplace_back(newPass);
    m_pGraph->Insert(newPass);
    newPass->mExecuteFunc = execute;

    RenderPassBuilder builder(*this, *newPass);
    setup(*this, builder);
    return newPass->GetHandle();
}
///////////RenderPassBuilder//////////////////

///////////TextureBuilder//////////////////
RenderGraph::TextureBuilder::TextureBuilder(RenderGraph& graph, TextureNode& textureNode)
: mGraph(graph), mTextureNode(textureNode)
{
    mTextureNode.mDesc.sample_count = GPU_SAMPLE_COUNT_1;
    mTextureNode.mDesc.descriptors  = GPU_RESOURCE_TYPE_TEXTURE;
    mTextureNode.mDesc.is_dedicated = false;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::Import(GPUTextureID texture, EGPUResourceState initedState)
{
    mTextureNode.m_pFrameTexture    = texture;
    mTextureNode.mInitState         = initedState;
    mTextureNode.mDesc.width        = texture->width;
    mTextureNode.mDesc.height       = texture->height;
    mTextureNode.mDesc.depth        = texture->depth;
    mTextureNode.mDesc.array_size   = texture->arraySizeMinusOne + 1;
    mTextureNode.mDesc.format       = (EGPUFormat)texture->format;
    mTextureNode.mDesc.sample_count = texture->sampleCount;
    mTextureNode.mImported          = texture;
    return *this;
}

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::SetName(const char* name)
{
    mTextureNode.SetName(name);
    return *this;
}

TextureHandle RenderGraph::CreateTexture(const TextureSetupFunc& setup)
{
    auto newTex = new TextureNode();
    mResources.emplace_back(newTex);
    m_pGraph->Insert(newTex);

    TextureBuilder builder(*this, *newTex);
    setup(*this, builder);
    return newTex->GetHandle();
}
///////////TextureBuilder//////////////////

RenderGraph::RenderGraph()
{

}
