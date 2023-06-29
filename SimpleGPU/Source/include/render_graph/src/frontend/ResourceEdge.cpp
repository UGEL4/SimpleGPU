#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"

///////////TextureEdge////////////////
TextureEdge::TextureEdge(ERelationshipType type, EGPUResourceState requestedState)
: RenderGraphEdge(type), mRequestedState(requestedState)
{

}
///////////TextureEdge////////////////

///////////TextureWriteEdge////////////////
TextureWriteEdge::TextureWriteEdge(TextureRTVHandle handle, EGPUResourceState requestedState)
: TextureEdge(ERelationshipType::TextureWrite, requestedState), mTextureHandle(handle)
{
    
}
PassNode* TextureWriteEdge::GetPassNode()
{
    return (PassNode*)From();
}
TextureNode* TextureWriteEdge::GetTextureNode()
{
    return (TextureNode*)To();
}
///////////TextureWriteEdge////////////////