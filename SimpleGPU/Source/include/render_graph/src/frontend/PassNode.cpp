#include "render_graph/include/frontend/PassNode.hpp"

const PassHandle PassNode::GetHandle() const
{
    return PassHandle(GetId());
}

PassNode::PassNode(EPassType type)
: RenderGraphNode(EObjectType::Pass)
{

}

RenderPassNode::RenderPassNode()
: PassNode(EPassType::Render)
{

}