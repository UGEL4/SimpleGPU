#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"

const PassHandle PassNode::GetHandle() const
{
    return PassHandle(GetId());
}

void PassNode::ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>& func)
{
    for (auto e : mOutTextureEdges)
    {
        func(e->GetTextureNode(), e);
    }
}

PassNode::PassNode(EPassType type)
: RenderGraphNode(EObjectType::Pass)
{

}

RenderPassNode::RenderPassNode()
: PassNode(EPassType::Render)
{

}