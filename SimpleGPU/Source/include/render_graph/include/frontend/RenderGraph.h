#pragma once
#include <functional>

class RenderGraph
{
public:
    using RenderGraphSetupFunc = std::function<void(void)>;
    static RenderGraph* Create(const RenderGraphSetupFunc& setup);
    static void Destroy(RenderGraph* graph);

    void Compile();
    virtual void Execute();

    RenderGraph();
    virtual ~RenderGraph() = default;
};