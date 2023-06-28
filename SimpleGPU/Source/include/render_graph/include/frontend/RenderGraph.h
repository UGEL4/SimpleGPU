#pragma once
#include <functional>
#include "render_graph/include/DependencyGraph.hpp"
#include "render_graph/include/frontend/BaseTypes.hpp"
#include <vector>

class PassNode;
class RenderGraph
{
public:
    using RenderGraphSetupFunc = std::function<void(void)>;
    static RenderGraph* Create(const RenderGraphSetupFunc& setup);
    static void Destroy(RenderGraph* graph);

    void Compile();
    virtual void Execute();
    virtual void Initialize();
    virtual void Finalize();

    using RenderPassSetupFunc = std::function<void(RenderGraph&)>;
    PassHandle AddRenderPass(const RenderPassSetupFunc& setup);

    RenderGraph();
    virtual ~RenderGraph() = default;

protected:
    DependencyGraph* m_pGraph = nullptr;
    std::vector<PassNode*> mPasses;
};