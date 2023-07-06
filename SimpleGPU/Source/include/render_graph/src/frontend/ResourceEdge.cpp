#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include "Utils.h"

///////////TextureEdge////////////////
TextureEdge::TextureEdge(ERelationshipType type, EGPUResourceState requestedState)
: RenderGraphEdge(type), mRequestedState(requestedState)
{

}
///////////TextureEdge////////////////

///////////TextureWriteEdge////////////////
TextureWriteEdge::TextureWriteEdge(uint32_t mrtIndex, TextureRTVHandle handle, EGPUResourceState requestedState)
: TextureEdge(ERelationshipType::TextureWrite, requestedState), mTextureHandle(handle), mMRTIndex(mrtIndex)
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

///////////TextureWriteEdge////////////////
TextureReadEdge::TextureReadEdge(const std::string_view& name, TextureSRVHandle handle, EGPUResourceState requestedState)
: TextureEdge(ERelationshipType::TextureRead, requestedState)
, mNameHash(GPUNameHash(name.data()))
, mName(name)
, mTextureHandle(handle)
{

}
PassNode* TextureReadEdge::GetPassNode()
{
    return (PassNode*)To();
}

TextureNode* TextureReadEdge::GetTextureNode()
{
    return (TextureNode*)From();
}
///////////TextureWriteEdge////////////////
///////////BufferEdge////////////////
BufferEdge::BufferEdge(ERelationshipType type, EGPUResourceState requestedState)
: RenderGraphEdge(type), mRequestedState(requestedState)
{

}

BufferReadEdge::BufferReadEdge(const std::string_view& name, BufferCBVHandle handle, EGPUResourceState requestedState)
: BufferEdge(ERelationshipType::BufferRead, requestedState), mHandle(handle)
{

}
PassNode* BufferReadEdge::GetPassNode()
{
    return (PassNode*)To();
}

BufferNode* BufferReadEdge::GetBufferNode()
{
    return (BufferNode*)From();
}
///////////BufferEdge////////////////
///////////BufferReadWriteEdge////////////////
BufferReadWriteEdge::BufferReadWriteEdge(const std::string_view& name, BufferUAVHandle handle, EGPUResourceState requestedState)
: BufferEdge(ERelationshipType::BufferReadWrite, requestedState), mHandle(handle)
{

}
PassNode* BufferReadWriteEdge::GetPassNode()
{
    return (PassNode*)To();
}

BufferNode* BufferReadWriteEdge::GetBufferNode()
{
    return (BufferNode*)From();
}
///////////BufferReadWriteEdge////////////////
