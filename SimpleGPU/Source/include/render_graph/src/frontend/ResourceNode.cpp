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
////////////////buffer node//////////////////////////
BufferNode::BufferNode()
: ResourceNode(EObjectType::Buffer)
{

}
BufferHandle BufferNode::GetHandle() const
{
    return BufferHandle(GetId());
}
////////////////buffer node//////////////////////////