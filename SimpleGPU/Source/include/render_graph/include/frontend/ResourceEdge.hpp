#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include "api.h"
#include <string_view>
#include <iostream>

class PassNode;
class TextureNode;
class TextureEdge : public RenderGraphEdge
{
    friend class RenderGraph;
    friend class RenderGraphBackend;
public:
    TextureEdge(ERelationshipType type, EGPUResourceState requestedState);
    virtual ~TextureEdge() = default;

    virtual PassNode* GetPassNode() = 0;
    virtual TextureNode* GetTextureNode() = 0;
protected:
    EGPUResourceState mRequestedState;
};

class TextureWriteEdge : public TextureEdge
{
public:
    TextureWriteEdge(TextureRTVHandle handle, EGPUResourceState requestedState = EGPUResourceState::GPU_RESOURCE_STATE_RENDER_TARGET);
    ~TextureWriteEdge() { std::cout << "Free TextureWriteEdge: from = " << mFromNode << ", to = " << mToNode << std::endl;}

    virtual PassNode* GetPassNode() final;
    virtual TextureNode* GetTextureNode() final;

private:
    TextureRTVHandle mTextureHandle;
};

class TextureReadEdge : public TextureEdge
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    TextureReadEdge(const std::string_view& name, TextureSRVHandle handle, EGPUResourceState requestedState = EGPUResourceState::GPU_RESOURCE_STATE_UNORDERED_ACCESS);
    virtual PassNode* GetPassNode() final;
    virtual TextureNode* GetTextureNode() final;

private:
    uint64_t mNameHash;
    std::string mName; // shader resource name
    TextureSRVHandle mTextureHandle;
};