#pragma once
#include <functional>
#include "render_graph/include/DependencyGraph.hpp"

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

    RenderGraph();
    virtual ~RenderGraph() = default;

protected:
    DependencyGraph* m_pGraph = nullptr;
};