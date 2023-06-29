#pragma once
#include "render_graph/include/frontend/BaseTypes.hpp"
#include "render_graph/include/DependencyGraph.hpp"
#include <vector>
#include <functional>

class TextureEdge;
class PassNode : public RenderGraphNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;

    PassHandle const GetHandle() const;
    void ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>&);
protected:
    PassNode(EPassType type);

protected:
    std::vector<TextureEdge*> mOutTextureEdges;
};

class RenderPassNode : public PassNode
{
public:
    friend class RenderGraph;
    friend class RenderGraphBackend;
    RenderPassNode();

private:
};