#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"

const PassHandle PassNode::GetHandle() const
{
    return PassHandle(GetId());
}

void PassNode::ForEachTextures(const std::function<void(TextureNode*, TextureEdge*)>& func)
{
    for (auto e : GetTextureReadEdges())
    {
        func(e->GetTextureNode(), e);
    }
    for (auto e : GetTextureWriteEdges())
    {
        func(e->GetTextureNode(), e);
    }
}

const bool PassNode::Before(const PassNode* other) const
{
    if (other == nullptr) return false;
    return (mOrder < other->mOrder);
}
const bool PassNode::After(const PassNode* other) const
{
    if (other == nullptr) return true;
    return (mOrder > other->mOrder);
}

std::span<TextureReadEdge*> PassNode::GetTextureReadEdges()
{
    return std::span<TextureReadEdge*>(mInTextureEdges.data(), mInTextureEdges.size());
}

std::span<TextureWriteEdge*> PassNode::GetTextureWriteEdges()
{
    return std::span<TextureWriteEdge*>(mOutTextureEdges.data(), mOutTextureEdges.size());
}

PassNode::PassNode(EPassType type, uint32_t order)
: RenderGraphNode(EObjectType::Pass), mOrder(order), mCanBeLone(false)
{

}

/////////////////////////////////////RenderPassNode
RenderPassNode::RenderPassNode(uint32_t order)
: PassNode(EPassType::Render, order)
{

}