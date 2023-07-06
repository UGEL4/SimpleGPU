#pragma once
#include <functional>
#include "render_graph/include/DependencyGraph.hpp"
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "api.h"
#include <vector>

class PassNode;
class RenderPassNode;
class ResourceNode;
class TextureEdge;
class PresentPassNode;
class BufferNode;

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
    virtual uint64_t Execute();
    virtual void Initialize();
    virtual void Finalize();

    EGPUResourceState GetLastestState(const TextureNode* texture, const PassNode* pending_pass);
    void ForeachWriterPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>&);
    void ForeachReaderPass(const TextureHandle handle, const std::function<void(TextureNode* texture, PassNode* pass, RenderGraphEdge* edge)>&);

    class RenderPassBuilder
    {
    public:
        friend class RenderGraph;
    protected:
        RenderPassBuilder(RenderGraph& graph, RenderPassNode& node);
    public:
        RenderPassBuilder& SetName(const char* name);
        RenderPassBuilder& Write(uint32_t mrtIndex, TextureRTVHandle handle, EGPULoadAction load, EGPUStoreAction store);
        RenderPassBuilder& Read(const char* name, TextureSRVHandle handle);
        RenderPassBuilder& SetRootSignature(GPURootSignatureID rs);
        RenderPassBuilder& SetPipeline(GPURenderPipelineID pipeline);
        RenderPassBuilder& SetDepthStencil(TextureDSVHandle handle, EGPULoadAction depthLoad, EGPUStoreAction depthStore, EGPULoadAction stencilLoad, EGPUStoreAction stencilStore);
        //pipeline
    private:
        RenderGraph& mGraph;
        RenderPassNode& mPassNode;
    };
    using RenderPassSetupFunc = std::function<void(RenderGraph&, RenderPassBuilder&)>;
    PassHandle AddRenderPass(const RenderPassSetupFunc& setup, const RenderPassExecuteFunction& execute);

    class TextureBuilder
    {
    public:
        friend class RenderGraph;
    protected:
        TextureBuilder(RenderGraph& graph, TextureNode& textureNode);
    public:
        TextureBuilder& Import(GPUTextureID texture, EGPUResourceState initedState);
        TextureBuilder& SetName(const char* name);
        TextureBuilder& AllowRenderTarget();
        //todo: all setters
    private:
        RenderGraph& mGraph;
        TextureNode& mTextureNode;
    };
    using TextureSetupFunc = std::function<void(RenderGraph&, TextureBuilder&)>;
    TextureHandle CreateTexture(const TextureSetupFunc& setup);

    class PresentPassBuilder
    {
    public:
        friend class RenderGraph;
        PresentPassBuilder& SetName(const char* name);
        PresentPassBuilder& Swapchain(GPUSwapchainID swapchain, uint32_t index);
        PresentPassBuilder& Texture(TextureHandle handle, bool isBackBuffer = true);
    protected:
        PresentPassBuilder(RenderGraph& graph, PresentPassNode& node);
    private:
        RenderGraph& mGraph;
        PresentPassNode& mPassNode;
    };
    using PresentPassSetupFunc = std::function<void(RenderGraph&, PresentPassBuilder&)>;
    PassHandle AddPresentPass(const PresentPassSetupFunc& setup);

    class BufferBuilder
    {
    public:
        friend class RenderGraph;
        BufferBuilder& SetrName(const char* name);
        BufferBuilder& Import(GPUBufferID buffer, EGPUResourceState initState);
        BufferBuilder& OwnsMemory();
        //BufferBuilder& structured(uint64_t first_element, uint64_t element_count, uint64_t element_stride) SKR_NOEXCEPT;
        BufferBuilder& Size(uint64_t size);
        BufferBuilder& WithFlags(GPUBufferCreationFlags flags);
        BufferBuilder& MemoryUsage(EGPUMemoryUsage mem_usage);
        //BufferBuilder& allow_shader_readwrite() SKR_NOEXCEPT;
        BufferBuilder& AllowShaderRead();
        BufferBuilder& AsUploadBuffer();
        BufferBuilder& AsVertexBuffer();
        BufferBuilder& AsIndexBuffer();
        BufferBuilder& AsUniformBuffer();
        BufferBuilder& PreferOnDevice();
        BufferBuilder& PreferOnHost();

    protected:
        BufferBuilder(RenderGraph& graph, BufferNode& node);
        RenderGraph& mGraph;
        BufferNode& mNode;
    };
    using BufferSetupFunc = std::function<void(RenderGraph&, BufferBuilder&)>;
    BufferHandle CreateBuffer(const BufferSetupFunc& setup);
    EGPUResourceState GetLastestState(const BufferNode* buffer, const PassNode* pending_pass);
    //BufferHandle GetBufferHandle(const char* name);

    RenderGraph();
    virtual ~RenderGraph() = default;

protected:
    DependencyGraph* m_pGraph = nullptr;
    struct NodeAndEdgeFactory* m_pNAEFactory = nullptr;
    std::vector<PassNode*> mPasses;
    std::vector<ResourceNode*> mResources;
    std::vector<PassNode*> mCulledPasses;
    std::vector<ResourceNode*> mCulledResources;
    uint32_t mFrameIndex = 0;
};

using RenderGraphBuilder = RenderGraph::RenderGraphBuilder;
using RenderPassBuilder  = RenderGraph::RenderPassBuilder;
using TextureBuilder     = RenderGraph::TextureBuilder;
using PresentPassBuilder = RenderGraph::PresentPassBuilder;
using BufferBuilder      = RenderGraph::BufferBuilder;