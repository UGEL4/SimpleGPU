#include "render_graph/include/backend/TextureViewPool.hpp"
#include "Utils.h"
#include "api.h"
#include <assert.h>
#include <utility>

namespace RG
{
    TextureViewPool::Key::operator size_t() const
    {
        return Utils::hash_val(this);
    }

    TextureViewPool::Key::Key(GPUDeviceID device, const GPUTextureViewDescriptor& desc)
    : m_pDevice(device), m_pTexture(desc.pTexture)
    {

    }

    void TextureViewPool::Initialize(GPUDeviceID device)
    {
        m_pDevice = device;
    }
    void TextureViewPool::Finalize()
    {
        for (auto&& pool : mTextureViews)
        {
            GPUFreeTextureView(pool.second.m_pTextureView);
        }
        mTextureViews.clear();
    }
    GPUTextureViewID TextureViewPool::Allocate(const GPUTextureViewDescriptor& desc, uint64_t frameIndex)
    {
        std::aligned_storage_t<sizeof(TextureViewPool::Key)> stroage;
        std::memset(&stroage, 0, sizeof(stroage));
        TextureViewPool::Key key = *(new (&stroage) TextureViewPool::Key(m_pDevice, desc));
        auto iter = mTextureViews.find(key);
        if (iter != mTextureViews.end())
        {
            assert(iter->second.m_pTextureView);
            iter->second.mMark.frame_index = frameIndex;
            return iter->second.m_pTextureView;
        }

        GPUTextureViewID view = GPUCreateTextureView(m_pDevice, &desc);
        AllocationMark mark = { frameIndex, 0 };
        //mTextureViews[key] = PooledTextureView(view, mark);
        mTextureViews.insert(std::make_pair(key, PooledTextureView(view, mark)));
        return view;
    }
}