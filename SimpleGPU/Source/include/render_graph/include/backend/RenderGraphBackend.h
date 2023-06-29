#pragma once

#include "render_graph/include/frontend/RenderGraph.h"
#include "api.h"

class RenderGraphBackend : public RenderGraph
{
public:
    RenderGraphBackend(const RenderGraphBuilder& builder);
    ~RenderGraphBackend() = default;

    virtual void Execute() override;
    virtual void Initialize() override;
    virtual void Finalize() override;

private:
    GPUDeviceID m_pDevice;
    GPUQueueID m_pQueue;
};