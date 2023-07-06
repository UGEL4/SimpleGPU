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

    const bool InImported() const { return mImported; }
protected:
    bool mImported = false;
};

class TextureNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    TextureNode();
    TextureHandle GetHandle() const;
    ~TextureNode() {std::cout << "free TextureNode :" << mId << std::endl;}

private:
    GPUTextureDescriptor mDesc = {};
    mutable GPUTextureID m_pFrameTexture;
    mutable EGPUResourceState mInitState = GPU_RESOURCE_STATE_UNDEFINED;
};

class BufferNode : public ResourceNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    BufferNode();
    BufferHandle GetHandle() const;
    ~BufferNode() {std::cout << "free BufferNode :" << mId << std::endl;}

private:
    GPUBufferDescriptor mDesc = {};
    mutable GPUBufferID m_pBuffer;
    mutable EGPUResourceState mInitState = GPU_RESOURCE_STATE_UNDEFINED;
};