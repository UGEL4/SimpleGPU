#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include <vector>
#include <functional>
#include <iostream>

class TextureEdge;
class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    PassHandle const GetHandle() const;
    void ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>&);
    ~PassNode() {std::cout << "Free PassNode : " << mId << std::endl;}

    uint32_t GetTextureCount() const { return (int32_t)mOutTextureEdges.size(); }
    const bool Before(const PassNode* other) const;
    const bool After(const PassNode* other) const;
protected:
    PassNode(EPassType type, uint32_t order);

protected:
    std::vector<TextureEdge*> mOutTextureEdges;
    uint32_t mOrder;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    RenderPassNode(uint32_t order);

private:
    RenderPassExecuteFunction mExecuteFunc;
};