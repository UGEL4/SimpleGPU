#pragma once

#include "render_graph/include/DependencyGraph.hpp"
#include "api.h"
#include <string>
#include <functional>

typedef uint64_t handle_t;
enum class EObjectType : uint8_t
{
    Pass,
    Texture,
    Buffer,
    Count
};

enum class EPassType : uint8_t
{
    None,
    Render,
    Compute,
    Copy,
    Present,
    Count
};

enum class ERelationshipType : uint8_t
{
    TextureRead,      // SRV
    TextureWrite,     // RTV/DSV
    TextureReadWrite, // UAV
    PipelineBuffer,   // VB/IB...
    BufferRead,       // CBV
    BufferReadWrite,  // UAV
    Count
};

template <EObjectType type>
struct ObjectHandle
{
    ObjectHandle(handle_t handle) : mHandle(handle){}

    inline operator handle_t() const { return mHandle; }

private:
    handle_t mHandle;
};

using PassHandle = ObjectHandle<EObjectType::Pass>;

template<>
struct ObjectHandle<EObjectType::Texture>
{
    struct ShaderWriteHandle
    {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureWriteEdge;
        ShaderWriteHandle(const handle_t _this) : mThis(_this) {}
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(mThis); }
    private:
        handle_t mThis;
    };
    inline operator ShaderWriteHandle() const { return ShaderWriteHandle(mHandle); }

    struct ShaderReadHandle
    {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;
        ShaderReadHandle(const handle_t _this) : mThis(_this) {}
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(mThis); }
    private:
        handle_t mThis;
    };
    inline operator ShaderReadHandle() const { return ShaderReadHandle(mHandle); }

    inline operator handle_t() const { return mHandle; }
    ObjectHandle() {}

    friend class TextureNode;
    friend class RenderGraph;
    friend class TextureRenderEdge;
protected:
    ObjectHandle(handle_t handle) : mHandle(handle){}
private:
    handle_t mHandle;
};
using TextureHandle    = ObjectHandle<EObjectType::Texture>;
using TextureRTVHandle = TextureHandle::ShaderWriteHandle;
using TextureSRVHandle = TextureHandle::ShaderReadHandle;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct RenderGraphNode : public DependencyGraphNode
{
    RenderGraphNode(EObjectType type) : type(type) {}
    void SetName(const char* name) { mName = name; }
    const char* GetName() const { return mName.c_str(); }
    const EObjectType type;
    const uint32_t pooled_size = 0;
protected:
    std::string mName = "";
};

struct RenderGraphEdge : public DependencyGraphEdge
{
    RenderGraphEdge(ERelationshipType type) : type(type) {}

    const ERelationshipType type;
    const uint32_t pooled_size = 0;
};

class PassNode;
class RenderGraphBackend;
class RenderGraph;
struct PassContext
{
    friend class RenderGraphBackend;
    PassNode* m_pPassNode;
    RenderGraphBackend* m_pGraph;
    GPUCommandBufferID m_pCmd;
};

struct RenderPassContext : public PassContext
{
    GPURenderPassEncoderID m_pEncoder;
};

using RenderPassExecuteFunction = std::function<void(RenderGraph&, RenderPassContext&)>;