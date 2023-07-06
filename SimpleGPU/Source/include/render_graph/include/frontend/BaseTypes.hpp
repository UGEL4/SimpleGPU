#pragma once

#include "render_graph/include/DependencyGraph.hpp"
#include "api.h"
#include <string>
#include <functional>

typedef uint64_t _handle_t;
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
    ObjectHandle(_handle_t handle) : mHandle(handle){}

    inline operator _handle_t() const { return mHandle; }

private:
    _handle_t mHandle;
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
        ShaderWriteHandle(const _handle_t _this) : mThis(_this) {}
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(mThis); }
        ShaderWriteHandle WriteMip(uint32_t level) const
        {
            ShaderWriteHandle handle = *this;
            handle.mMipBase          = level;
            return handle;
        }
        ShaderWriteHandle WriteArray(uint32_t base, uint32_t count) const
        {
            ShaderWriteHandle handle = *this;
            handle.mArrayBase        = base;
            handle.mArrayCount       = count;
            return handle;
        }
    private:
        _handle_t mThis;
        uint32_t mMipBase     = 0;
        uint32_t mArrayBase   = 0;
        uint32_t mArrayCount  = 1;
    };
    inline operator ShaderWriteHandle() const { return ShaderWriteHandle(mHandle); }

    struct DepthStencilHandle : public ShaderWriteHandle
    {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureWriteEdge;
        DepthStencilHandle(const _handle_t _this) : ShaderWriteHandle(_this) {}
        DepthStencilHandle ClearDepth(float depth) const
        {
            DepthStencilHandle handle = *this;
            handle.mClearDepth = depth;
            return handle;
        }
    private:
        float mClearDepth = 0.f;
    };
    inline operator DepthStencilHandle() const { return DepthStencilHandle(mHandle); }

    struct ShaderReadHandle
    {
        friend struct ObjectHandle<EObjectType::Texture>;
        friend class RenderGraph;
        friend class TextureReadEdge;
        ShaderReadHandle(const _handle_t _this, uint32_t mipBase = 0, uint32_t mipCount = 1, uint32_t arrayBase = 0, uint32_t arrayCount = 1)
        : mThis(_this), mMipBase(mipBase), mMipCount(mipCount), mArrayBase(arrayBase), mArrayCount(arrayCount)
        {

        }
        inline operator ObjectHandle<EObjectType::Texture>() const { return ObjectHandle<EObjectType::Texture>(mThis); }
        ShaderReadHandle ReadMip(uint32_t base, uint32_t count) const
        {
            ShaderReadHandle handle = *this;
            handle.mMipBase         = base;
            handle.mMipCount        = count;
            return handle;
        }

        ShaderReadHandle ReadArray(uint32_t base, uint32_t count) const
        {
            ShaderReadHandle handle = *this;
            handle.mArrayBase       = base;
            handle.mArrayCount      = count;
            return handle;
        }

        ShaderReadHandle ReadDimension(EGPUTextureDimension dim) const
        {
            ShaderReadHandle handle = *this;
            handle.mDim              = dim;
            return handle;
        }
    private:
        _handle_t mThis;
        uint32_t mMipBase        = 0;
        uint32_t mMipCount       = 1;
        uint32_t mArrayBase      = 0;
        uint32_t mArrayCount     = 1;
        EGPUTextureDimension mDim = GPU_TEX_DIMENSION_2D;
    };
    inline operator ShaderReadHandle() const { return ShaderReadHandle(mHandle); }

    inline operator _handle_t() const { return mHandle; }
    ObjectHandle() {}

    friend class TextureNode;
    friend class RenderGraph;
    friend class TextureRenderEdge;
    friend class TextureReadEdge;
protected:
    ObjectHandle(_handle_t handle) : mHandle(handle){}
private:
    _handle_t mHandle;
};
using TextureHandle    = ObjectHandle<EObjectType::Texture>;
using TextureRTVHandle = TextureHandle::ShaderWriteHandle;
using TextureSRVHandle = TextureHandle::ShaderReadHandle;
using TextureDSVHandle = TextureHandle::DepthStencilHandle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<>
struct ObjectHandle<EObjectType::Buffer>
{
    struct ShaderReadHandle
    {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadEdge;
        ShaderReadHandle(const _handle_t _this)
        : mThis(_this)
        {

        }
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(mThis); }

    private:
        _handle_t mThis;
    };
    inline operator ShaderReadHandle() const { return ShaderReadHandle(mHandle); }

    struct ShaderReadWriteHandle
    {
        friend struct ObjectHandle<EObjectType::Buffer>;
        friend class RenderGraph;
        friend class BufferReadEdge;
        ShaderReadWriteHandle(const _handle_t _this)
        : mThis(_this)
        {

        }
        inline operator ObjectHandle<EObjectType::Buffer>() const { return ObjectHandle<EObjectType::Buffer>(mThis); }

    private:
        _handle_t mThis;
    };
    inline operator ShaderReadWriteHandle() const { return ShaderReadWriteHandle(mHandle); }

    inline operator _handle_t() const { return mHandle; }
    ObjectHandle() {}

    friend class BufferNode;
    friend class RenderGraph;
    friend class BufferReadEdge;
    friend class BufferReadWriteEdge;
protected:
    ObjectHandle(_handle_t handle) : mHandle(handle){}
private:
    _handle_t mHandle;
};

using BufferHandle = ObjectHandle<EObjectType::Buffer>;
using BufferCBVHandle = ObjectHandle<EObjectType::Buffer>::ShaderReadHandle;
using BufferUAVHandle = ObjectHandle<EObjectType::Buffer>::ShaderReadWriteHandle;

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
    const struct GPUBindTable* m_pBindTable;
};

using RenderPassExecuteFunction = std::function<void(RenderGraph&, RenderPassContext&)>;