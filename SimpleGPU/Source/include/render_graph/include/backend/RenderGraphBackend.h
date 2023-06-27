#pragma once

#include "render_graph/include/frontend/RenderGraph.h"

class RenderGraphBackend : public RenderGraph
{
public:
    RenderGraphBackend();
    ~RenderGraphBackend() = default;

    virtual void Execute() override;
    virtual void Initialize() override;
    virtual void Finalize() override;
};