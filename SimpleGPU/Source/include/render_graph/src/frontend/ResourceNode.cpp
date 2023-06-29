#include "render_graph/include/frontend/ResourceNode.hpp"

ResourceNode::ResourceNode(EObjectType type)
: RenderGraphNode(type)
{

}

////////////////texture node//////////////////////////
TextureNode::TextureNode()
: ResourceNode(EObjectType::Texture)
{

}

TextureHandle TextureNode::GetHandle() const
{
    return TextureHandle(GetId());
}
////////////////texture node//////////////////////////