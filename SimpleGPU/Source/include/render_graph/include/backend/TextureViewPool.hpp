#pragma once

#include <unordered_map>
#include <deque>
#include "api.h"

namespace RG
{
    class TextureViewPool
    {
    public:
        struct AllocationMark
        {
            uint64_t frame_index;
            uint32_t tags;
        };
        struct PooledTextureView
        {
            PooledTextureView() = delete;
            PooledTextureView(GPUTextureViewID texture, AllocationMark mark)
                : m_pTextureView(texture), mMark(mark)
            {

            }
            GPUTextureViewID m_pTextureView;
            AllocationMark mMark;
        };
        struct Key
        {
            const GPUDeviceID m_pDevice;
            const GPUTextureID m_pTexture;
            operator size_t() const;
            friend class TextureViewPool;
            struct hasher
            {
                inline size_t operator()(const Key& val) const { return (size_t)val; }
            };

            Key(GPUDeviceID device, const GPUTextureViewDescriptor& desc);
        };
        friend class RenderGraphBackend;
        void Initialize(GPUDeviceID device);
        void Finalize();
        GPUTextureViewID Allocate(const GPUTextureViewDescriptor& desc, uint64_t frameIndex);

    protected:
        GPUDeviceID m_pDevice;
        std::unordered_map<Key, PooledTextureView, Key::hasher> mTextureViews;
    };
}