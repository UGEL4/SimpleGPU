#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include "api.h"
#include <iostream>

class ResourceNode : public RenderGraphNode
{
public:
    ResourceNode(EObjectType type);
    virtual ~ResourceNode() = default;

protected:
    bool mInported = false;
};

class TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    TextureNode();
    TextureHandle GetHandle() const;
    ~TextureNode() {std::cout << "free TextureNode :" << mId << std::endl;}

private:
    GPUTextureDescriptor mDesc = {};
    mutable GPUTextureID m_pFrameTexture;
    EGPUResourceState mInitState;
};