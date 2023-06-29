#pragma once
#include <functional>
#include "render_graph/include/DependencyGraph.hpp"
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "api.h"
#include <vector>

class PassNode;
class RenderPassNode;
class ResourceNode;
class RenderGraph
{
public:
    class RenderGraphBuilder
    {
    public:
        friend class RenderGraphBackend;
        RenderGraphBuilder& WithDevice(GPUDeviceID device);
        RenderGraphBuilder& WithGFXQueue(GPUQueueID queue);
    private:
        GPUDeviceID m_pDevice;
        GPUQueueID m_pQueue;
    };
    using RenderGraphSetupFunc = std::function<void(RenderGraphBuilder&)>;
    static RenderGraph* Create(const RenderGraphSetupFunc& setup);
    static void Destroy(RenderGraph* graph);

    void Compile();
    virtual void Execute();
    virtual void Initialize();
    virtual void Finalize();

    class RenderPassBuilder
    {
    public:
        friend class RenderGraph;
    protected:
        RenderPassBuilder(RenderGraph& graph, RenderPassNode& node);
    public:
        RenderPassBuilder& Write(TextureRTVHandle handle);
    private:
        RenderGraph& mGraph;
        RenderPassNode& mPassNode;
    };
    using RenderPassSetupFunc = std::function<void(RenderGraph&, RenderPassBuilder&)>;
    PassHandle AddRenderPass(const RenderPassSetupFunc& setup);

    class TextureBuilder
    {
    public:
        friend class RenderGraph;
    protected:
        TextureBuilder(RenderGraph& graph, TextureNode& textureNode);
    public:
        TextureBuilder& Import(GPUTextureID texture, EGPUResourceState initedState);
    private:
        RenderGraph& mGraph;
        TextureNode& mTextureNode;
    };
    using TextureSetupFunc = std::function<void(RenderGraph&, TextureBuilder&)>;
    TextureHandle CreateTexture(const TextureSetupFunc& setup);

    RenderGraph();
    virtual ~RenderGraph() = default;

protected:
    DependencyGraph* m_pGraph = nullptr;
    std::vector<PassNode*> mPasses;
    std::vector<ResourceNode*> mResources;
};

using RenderGraphBuilder = RenderGraph::RenderGraphBuilder;
using RenderPassBuilder  = RenderGraph::RenderPassBuilder;
using TextureBuilder     = RenderGraph::TextureBuilder;