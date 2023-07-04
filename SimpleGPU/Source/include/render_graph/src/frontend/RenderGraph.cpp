#include "render_graph/include/frontend/RenderGraph.h"
#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/NodeAndEdgeFactory.hpp"
#include <iostream>
#include <assert.h>
#include <stdint.h>

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
    mPasses.erase(
        std::remove_if(mPasses.begin(), mPasses.end(), [this](PassNode* pass)
        {
            bool lone = !(pass->InComingEdges() + pass->OutGoingEdges());
            if (lone && !pass->mCanBeLone) mCulledPasses.emplace_back(pass);
            return lone;
        }),
        mPasses.end()
    );

    mResources.erase(
        std::remove_if(mResources.begin(), mResources.end(), [this](ResourceNode* node)
        {
            bool lone = !(node->InComingEdges() + node->OutGoingEdges());
            if (lone) mCulledResources.emplace_back(node);
            return lone;
        }),
        mResources.end()
    );
}

uint64_t RenderGraph::Execute()
{
    std::cout << "RenderGraph::Execute" << std::endl;
    m_pGraph->Clear();
    return mFrameIndex++;
}

void RenderGraph::Initialize()
{
    m_pGraph      = DependencyGraph::Create();
    m_pNAEFactory = NodeAndEdgeFactory::Create();
}

void RenderGraph::Finalize()
{
    NodeAndEdgeFactory::Destroy(m_pNAEFactory);
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

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Write(uint32_t mrtIndex, TextureRTVHandle handle, EGPULoadAction load, EGPUStoreAction store)
{
    //链接pass_node 和 resource_node
    TextureWriteEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureWriteEdge>(mrtIndex, handle);
    mPassNode.mOutTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(handle.mThis), edge);
    mPassNode.mLoadActions[mrtIndex]  = load;
    mPassNode.mStoreActions[mrtIndex] = store;
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::Read(const char* name, TextureSRVHandle handle)
{
    TextureReadEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureReadEdge>(name, handle);
    mPassNode.mInTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(handle.mThis), &mPassNode, edge);
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetRootSignature(GPURootSignatureID rs)
{
    mPassNode.m_pRootSignature = rs;
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetPipeline(GPURenderPipelineID pipeline)
{
    mPassNode.m_pPipeline      = pipeline;
    mPassNode.m_pRootSignature = pipeline->pRootSignature;
    return *this;
}

RenderGraph::RenderPassBuilder& RenderGraph::RenderPassBuilder::SetDepthStencil(TextureDSVHandle handle,
                                                                                EGPULoadAction depthLoad, EGPUStoreAction depthStore,
                                                                                EGPULoadAction stencilLoad, EGPUStoreAction stencilStore)
{
    TextureWriteEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureWriteEdge>(GPU_MAX_MRT_COUNT, handle, GPU_RESOURCE_STATE_DEPTH_WRITE);
    mPassNode.mOutTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(&mPassNode, mGraph.m_pGraph->AccessNode(handle.mThis), edge);
    mPassNode.mDepthLoadAction    = depthLoad;
    mPassNode.mDepthStoreAction   = depthStore;
    mPassNode.mStencilLoadAction  = stencilLoad;
    mPassNode.mStencilStoreAction = stencilStore;
    mPassNode.mClearDepth         = handle.mClearDepth;
    return *this;
}

PassHandle RenderGraph::AddRenderPass(const RenderPassSetupFunc& setup, const RenderPassExecuteFunction& execute)
{
    uint32_t order          = (uint32_t)mPasses.size();
    RenderPassNode* newPass = m_pNAEFactory->Allocate<RenderPassNode>(order);
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

RenderGraph::TextureBuilder& RenderGraph::TextureBuilder::AllowRenderTarget()
{
    mTextureNode.mDesc.descriptors |= GPU_RESOURCE_TYPE_RENDER_TARGET;
    mTextureNode.mDesc.start_state = GPU_RESOURCE_STATE_UNDEFINED;
    return *this;
}

TextureHandle RenderGraph::CreateTexture(const TextureSetupFunc& setup)
{
    auto newTex = m_pNAEFactory->Allocate<TextureNode>();
    mResources.emplace_back(newTex);
    m_pGraph->Insert(newTex);

    TextureBuilder builder(*this, *newTex);
    setup(*this, builder);
    return newTex->GetHandle();
}
///////////TextureBuilder//////////////////

///////////PresentPassBuilder//////////////////
RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::SetName(const char* name)
{
    mPassNode.SetName(name);
    return *this;
}

RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::Swapchain(GPUSwapchainID swapchain, uint32_t index)
{
    mPassNode.mDesc.swapchain = swapchain;
    mPassNode.mDesc.index     = index;
    return *this;
}

RenderGraph::PresentPassBuilder& RenderGraph::PresentPassBuilder::Texture(TextureHandle handle, bool isBackBuffer)
{
    assert(isBackBuffer);
    TextureReadEdge* edge = mGraph.m_pNAEFactory->Allocate<TextureReadEdge>("PresentSrc", handle, GPU_RESOURCE_STATE_PRESENT);
    mPassNode.mInTextureEdges.emplace_back(edge);
    mGraph.m_pGraph->Link(mGraph.m_pGraph->AccessNode(handle), &mPassNode, edge);
    return *this;
}

RenderGraph::PresentPassBuilder::PresentPassBuilder(RenderGraph& graph, PresentPassNode& node)
: mGraph(graph), mPassNode(node)
{

}

PassHandle RenderGraph::AddPresentPass(const PresentPassSetupFunc& setup)
{
    uint32_t size = (uint32_t)mPasses.size();
    PresentPassNode* node = m_pNAEFactory->Allocate<PresentPassNode>(size);
    mPasses.emplace_back(node);
    m_pGraph->Insert(node);

    PresentPassBuilder builder(*this, *node);
    setup(*this, builder);
    return node->GetHandle();
}

///////////PresentPassBuilder//////////////////

RenderGraph::RenderGraph()
{

}
