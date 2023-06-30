#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include "api.h"
#include <iostream>

class PassNode;
class TextureNode;
class TextureEdge : public RenderGraphEdge
{
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