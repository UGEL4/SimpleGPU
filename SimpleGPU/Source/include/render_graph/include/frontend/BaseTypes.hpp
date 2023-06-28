#pragma once

#include "render_graph/include/DependencyGraph.hpp"
#include <string>

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

template <EObjectType type>
struct ObjectHandle
{
    ObjectHandle(handle_t handle) : mHandle(handle){}

    inline operator handle_t() const { return mHandle; }
    handle_t mHandle;
};

using PassHandle = ObjectHandle<EObjectType::Pass>;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct RenderGraphNode : public DependencyGraphNode
{
    RenderGraphNode(EObjectType type) : type(type){}
    /* SKR_RENDER_GRAPH_API void set_name(const char8_t* n);
    SKR_RENDER_GRAPH_API const char8_t* get_name() const; */
    const EObjectType type;
    const uint32_t pooled_size = 0;
protected:
    //graph_object_string name = u8"";
};