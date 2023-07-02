#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include <iostream>
#include <stdint.h>
#include <vector>
#include <assert.h>

//////////////////RenderGraphFrameExecutor////////////////////////
void RenderGraphFrameExecutor::Initialize(GPUDeviceID gfxDevice, GPUQueueID gfxQueue)
{
    m_pCommandPool                  = GPUCreateCommandPool(gfxQueue);
    GPUCommandBufferDescriptor desc = {};
    desc.isSecondary                = false;
    m_pCmd                          = GPUCreateCommandBuffer(m_pCommandPool, &desc);
    m_pFence                        = GPUCreateFence(gfxDevice);
}

void RenderGraphFrameExecutor::Finalize()
{
    if (m_pCmd) GPUFreeCommandBuffer(m_pCmd);
    if (m_pCommandPool) GPUFreeCommandPool(m_pCommandPool);
    if (m_pFence) GPUFreeFence(m_pFence);
    m_pCommandPool = nullptr;
    m_pCmd         = nullptr;
    m_pFence       = nullptr;
}
//////////////////RenderGraphFrameExecutor////////////////////////

//////////////////RenderGraphBackend////////////////////////
RenderGraphBackend::RenderGraphBackend(const RenderGraphBuilder& builder)
: m_pDevice(builder.m_pDevice), m_pQueue(builder.m_pQueue)
{

}

void RenderGraphBackend::Execute()
{
    std::cout << "RenderGraphBackend::Execute" << std::endl;

    uint32_t frameIndex = mFrameIndex % RG_MAX_FRAME_IN_FILGHT;
    auto& executor = mExecutors[frameIndex];
    GPUWaitFences(&executor.m_pFence, 1);

    GPUCmdBegin(executor.m_pCmd);
    {
        for (auto& pass : mPasses)
        {
            if (pass->type == EObjectType::Pass)
            {
                ExecuteRenderPass(static_cast<RenderPassNode*>(pass), executor);
            }
        }
    }
    GPUCmdEnd(executor.m_pCmd);

    //clear
    {
        //pass
        for (auto pass : mPasses)
        {
            if (pass)
            {
                pass->ForEachTextures([this](TextureNode* texture, TextureEdge* edge)
                {
                    if (edge) delete edge;
                });
                delete pass;
            }
        }
        mPasses.clear();

        //resources
        for (auto res : mResources)
        {
            if (res)
            {
                delete res;
            }
        }
        mResources.clear();

        m_pGraph->Clear();
    }
}

void RenderGraphBackend::Initialize()
{
    RenderGraph::Initialize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FILGHT; i++)
    {
        mExecutors[i].Initialize(m_pDevice, m_pQueue);
    }
    mTexturePool.Initialize(m_pDevice);
    mTextureViewPool.Initialize(m_pDevice);
}

void RenderGraphBackend::Finalize()
{
    RenderGraph::Finalize();
    for (uint32_t i = 0; i < RG_MAX_FRAME_IN_FILGHT; i++)
    {
        mExecutors[i].Finalize();
    }
}

void RenderGraphBackend::ExecuteRenderPass(RenderPassNode* pass,  RenderGraphFrameExecutor& executor)
{
    // resource de-virtualize
    std::vector<GPUTextureBarrier> tex_barriers;
    std::vector<std::pair<TextureHandle, GPUTextureID>> resolved_textures;
    CalculateResourceBarriers(executor, pass, tex_barriers, resolved_textures);
    //alloca & update descriptorset
    RenderPassContext passContext {};
    passContext.m_pPassNode = pass;
    passContext.m_pCmd      = executor.m_pCmd;
    passContext.m_pGraph    = this;
    AllocateAndUpdatePassBindTable(executor, pass, pass->m_pRootSignature);
    //call gpu aip
    //deallace resource
}

void RenderGraphBackend::CalculateResourceBarriers(RenderGraphFrameExecutor& executor, PassNode* pass,
        std::vector<GPUTextureBarrier>& tex_barriers, std::vector<std::pair<TextureHandle, GPUTextureID>>& resolved_textures)
{
    tex_barriers.resize(pass->GetTextureCount());
    resolved_textures.resize(pass->GetTextureCount());
    //遍历pass的每一个texture资源
    pass->ForEachTextures([&](TextureNode* tex, TextureEdge* edge)
    {
        //分配texture资源
        auto resolved_texture = Resolve(executor, *tex);
        resolved_textures.emplace_back(tex->GetHandle(), resolved_texture);
        auto curr_state = GetLastestState(tex, pass);
        if (curr_state == edge->mRequestedState) return;
        //分配barrier
        GPUTextureBarrier barrier{};
        barrier.texture   = resolved_texture;
        barrier.src_state = curr_state;
        barrier.dst_state = edge->mRequestedState;
        tex_barriers.emplace_back(barrier);
    });

    //遍历pass的每一个buffer资源
}

GPUTextureID RenderGraphBackend::Resolve(RenderGraphFrameExecutor& executor, const TextureNode& texture)
{
    if (!texture.m_pFrameTexture)
    {
        //allocate texture
    }
    return texture.m_pFrameTexture;
}

GPUBindTableID RenderGraphBackend::AllocateAndUpdatePassBindTable(RenderGraphFrameExecutor& executor, PassNode* pass, GPURootSignatureID root_sig)
{
    if (root_sig == nullptr) return nullptr;
    GPUBindTable* table = nullptr;
    {
        auto texReadEdges = pass->GetTextureReadEdges();

        // Allocate or get descriptor set heap
        auto iter = executor.mBindTablePools.find(root_sig);
        if (iter == executor.mBindTablePools.end())
        {
            void* ptr           = calloc(1, sizeof(BindTablePool));
            BindTablePool* pool = new (ptr) BindTablePool(root_sig);
            executor.mBindTablePools.emplace(root_sig, pool);
        }
        // Bind resources
        std::string bind_table_keys = "";
        std::vector<GPUDescriptorData> desc_set_updates;
        std::vector<const char*> bindTableValueNames = {};
        std::vector<GPUTextureViewID> SRVs(texReadEdges.size());
        for (uint32_t i = 0; i < texReadEdges.size(); i++)
        {
            auto& readEdge = texReadEdges[i];
            assert(!readEdge->mName.empty());
            const auto& res = *FindShaderResource(readEdge->mNameHash, root_sig);
            bind_table_keys += readEdge->mName.empty() ? (const char*)res.name : readEdge->mName;
            bind_table_keys += ';';
            bindTableValueNames.emplace_back(bind_table_keys.c_str());

            auto texture_readed = readEdge->GetTextureNode();
            GPUDescriptorData update = {};
            update.count = 1;
            update.name = res.name;
            update.binding_type = GPU_RESOURCE_TYPE_TEXTURE;
            update.binding = res.binding;
            GPUTextureViewDescriptor view_desc = {};
            view_desc.pTexture                 = Resolve(executor, *texture_readed);
            view_desc.baseArrayLayer           = readEdge->GetArrayBase();
            view_desc.arrayLayerCount          = readEdge->GetArrayCount();
            view_desc.baseMipLevel             = readEdge->GetMipBase();
            view_desc.mipLevelCount            = readEdge->GetMipCount();
            view_desc.format                   = (EGPUFormat)view_desc.pTexture->format;
            //const bool is_depth_stencil = FormatUtil_IsDepthStencilFormat(view_desc.format);
            bool is_depth_stencil = false;
            switch (view_desc.format)
            {
                case GPU_FORMAT_D24_UNORM_S8_UINT:
                case GPU_FORMAT_D32_SFLOAT_S8_UINT:
                case GPU_FORMAT_D16_UNORM_S8_UINT:
                    is_depth_stencil = true;
                    break;
            }
            //const bool is_depth_only = FormatUtil_IsDepthStencilFormat(view_desc.format);
            const bool is_depth_only = is_depth_stencil;
            view_desc.aspectMask =
                is_depth_stencil ?
                is_depth_only ? GPU_TVA_DEPTH : GPU_TVA_DEPTH | GPU_TVA_STENCIL :
                GPU_TVA_COLOR;
            view_desc.usages = GPU_TVU_SRV;
            //view_desc.dims = read_edge->get_dimension();
            SRVs[i] = mTextureViewPool.Allocate(view_desc, mFrameIndex);
            update.textures = &SRVs[i];
            desc_set_updates.emplace_back(update);
        }
    }
    return table;
}

const GPUShaderResource* RenderGraphBackend::FindShaderResource(uint64_t nameHash, GPURootSignatureID rs, EGPUResourceType* type) const
{
    for (uint32_t i = 0; i < rs->table_count; i++)
        {
            for (uint32_t j = 0; j < rs->tables[i].resources_count; j++)
            {
                const auto& resource = rs->tables[i].resources[j];
                if (resource.name_hash == nameHash)
                //if (strcmp((const char*)resource.name, name.c_str()) == 0)
                {
                    if (type) *type = resource.type;
                    return &rs->tables[i].resources[j];
                }
            }
        }
        return nullptr;
}
//////////////////RenderGraphBackend////////////////////////