#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include "api.h"

class ResourceNode : public RenderGraphNode
{
public:
    ResourceNode(EObjectType type);
};

class TextureNode : public ResourceNode
{
public:
    TextureNode();
    TextureHandle GetHandle() const;

private:
    GPUTextureDescriptor mDesc = {};
    mutable GPUTextureID m_pFrameTexture;
    EGPUResourceState mInitState;
};