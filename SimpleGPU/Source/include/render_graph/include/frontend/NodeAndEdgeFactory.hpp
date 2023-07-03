#pragma once
#include <stdlib.h>
#include "BaseTypes.hpp"

struct NodeAndEdgeFactory
{
    virtual ~NodeAndEdgeFactory() = default;
    static NodeAndEdgeFactory* Create();
    static void Destroy(NodeAndEdgeFactory* factory);

    template<typename T, typename... Args>
    T* Allocate(Args&&... args)
    {
        if (auto allocated = InternalAlloc<T>())
        {
            new (allocated) T(std::forward<Args>(args)...);
            auto& pooled_size = const_cast<uint32_t&>(allocated->pooled_size);
            pooled_size = (uint32_t)sizeof(T);
            return allocated;
        }
        void* ptr = calloc(1, sizeof(T));
        T* newT   = new (ptr) T(std::forward<Args>(args)...);
        return newT;
    }

    template<typename T>
    void Dealloc(T* object)
    {
        if (InternalFree<T>(object)) return;
        if (object)
        {
            object->~T();
            free(object);
        }
    }

    template<typename T>
    T* InternalAlloc()
    {
        return (T*)internalAllocateMemory(sizeof(T));
    }
    
    template<typename T>
    bool InternalFree(T* object)
    {
        if (object->pooled_size)
        {
            object->~T();
            return internalFreeMemory(object, object->pooled_size);
        }
        if (object)
        {
            object->~T();
            free(object);
        }
        return true;
    }

    virtual bool InternalFreeMemory(void* memory, size_t size) = 0;
    virtual void* InternalAllocateMemory(size_t size) = 0;
}; 