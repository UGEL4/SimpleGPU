#include "render_graph/include/backend/RenderGraphBackend.h"
#include "render_graph/include/frontend/PassNode.hpp"
#include "render_graph/include/frontend/ResourceEdge.hpp"
#include "render_graph/include/frontend/ResourceNode.hpp"
#include "Utils.h"
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
    for (auto iter : mBindTablePools)
    {
        if (iter.second)
        {
            iter.second->Destroy();
            iter.second->~BindTablePool();
            free(iter.second);
        }
    }
    mBindTablePools.clear();
}

void RenderGraphFrameExecutor::ResetOnStart()
{
    for (auto iter : mBindTablePools)
    {
        if (iter.second) iter.second->Reset();
    }
    GPUResetCommandPool(m_pCommandPool);
}
//////////////////RenderGraphFrameExecutor////////////////////////

//////////////////RenderGraphBackend////////////////////////
RenderGraphBackend::RenderGraphBackend(const RenderGraphBuilder& builder)
: m_pDevice(builder.m_pDevice), m_pQueue(builder.m_pQueue)
{

}

uint64_t RenderGraphBackend::Execute()
{
    std::cout << "RenderGraphBackend::Execute" << std::endl;

    uint32_t frameIndex = mFrameIndex % RG_MAX_FRAME_IN_FILGHT;
    auto& executor = mExecutors[frameIndex];
    GPUWaitFences(&executor.m_pFence, 1);

    executor.ResetOnStart();
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

    return mFrameIndex++;
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
    mTextureViewPool.Finalize();
    mTexturePool.Finalize();
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
    passContext.m_pBindTable = AllocateAndUpdatePassBindTable(executor, pass, pass->m_pRootSignature);
    //call gpu aip
    GPUResourceBarrierDescriptor barrier_desc{};
    if (!tex_barriers.empty())
    {
        barrier_desc.texture_barriers = tex_barriers.data();
        barrier_desc.texture_barriers_count = (uint32_t)tex_barriers.size();
    }
    GPUCmdResourceBarrier(executor.m_pCmd, &barrier_desc);

    {
        std::vector<GPUColorAttachment> attachments = {};
        GPUDepthStencilAttachment ds_attachment     = {};
        auto writeEdges                             = pass->GetTextureWriteEdges();
        EGPUSampleCount passSampleCount             = GPU_SAMPLE_COUNT_1;
        for (auto edge : writeEdges)
        {
            auto targetTexture = edge->GetTextureNode();
            bool is_depth_stencil = Utils::FormatUtil_IsDepthStencilFormat((EGPUFormat)(Resolve(executor, *targetTexture)->format));
            bool is_depth_only = Utils::FormatUtil_IsDepthOnlyFormat((EGPUFormat)(Resolve(executor, *targetTexture)->format));
            if (edge->mRequestedState == GPU_RESOURCE_STATE_DEPTH_WRITE && is_depth_stencil)
            {
                GPUTextureViewDescriptor desc {};
                desc.pTexture        = Resolve(executor, *targetTexture);
                desc.format          = (EGPUFormat)desc.pTexture->format;
                desc.usages          = GPU_TVU_RTV_DSV;
                desc.aspectMask      = is_depth_only ? GPU_TVA_DEPTH : GPU_TVA_DEPTH | GPU_TVA_STENCIL;
                desc.baseMipLevel    = edge->GetMipBase();
                desc.mipLevelCount   = 1;
                desc.baseArrayLayer  = edge->GetArrayBase();
                desc.arrayLayerCount = edge->GetArrayCount();

                ds_attachment.view                 = mTextureViewPool.Allocate(desc, mFrameIndex);
                ds_attachment.depth_load_action    = pass->mDepthLoadAction;
                ds_attachment.depth_store_action   = pass->mDepthStoreAction;
                ds_attachment.clear_depth          = pass->mClearDepth;
                ds_attachment.write_depth          = 1;
                ds_attachment.stencil_load_action  = pass->mStencilLoadAction;
                ds_attachment.stencil_store_action = pass->mStencilStoreAction;
            }
            else
            {
                GPUTextureViewDescriptor desc {};
                desc.pTexture        = Resolve(executor, *targetTexture);
                desc.format          = (EGPUFormat)desc.pTexture->format;
                desc.usages          = GPU_TVU_RTV_DSV;
                desc.aspectMask      = GPU_TVA_COLOR;
                desc.baseMipLevel    = edge->GetMipBase();
                desc.mipLevelCount   = 1;
                desc.baseArrayLayer  = edge->GetArrayBase();
                desc.arrayLayerCount = edge->GetArrayCount();

                GPUColorAttachment colorAttachment{};
                colorAttachment.view         = mTextureViewPool.Allocate(desc, mFrameIndex);
                colorAttachment.load_action  = pass->mLoadActions[edge->mMRTIndex];
                colorAttachment.store_action = pass->mStoreActions[edge->mMRTIndex];
                colorAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };//todo : write edge store clear_color

                attachments.emplace_back(colorAttachment);
            }
        }
        GPURenderPassDescriptor render_pass_desc{};
        render_pass_desc.name                = pass->GetName();
        render_pass_desc.sample_count        = passSampleCount;
        render_pass_desc.color_attachments   = attachments.data();
        render_pass_desc.render_target_count = (uint32_t)attachments.size();
        render_pass_desc.depth_stencil       = &ds_attachment;
        passContext.m_pEncoder = GPUCmdBeginRenderPass(executor.m_pCmd, &render_pass_desc);
        {
            if (pass->m_pPipeline) GPURenderEncoderBindPipeline(passContext.m_pEncoder, pass->m_pPipeline);
            if (passContext.m_pEncoder) GPURenderEncoderBindBindTable(passContext.m_pEncoder, passContext.m_pBindTable);
            pass->mExecuteFunc(*this, passContext);
        }
        GPUCmdEndRenderPass(executor.m_pCmd, passContext.m_pEncoder);
    }
    //deallace resource
    DeallocaResources(pass);
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
        auto allocated          = mTexturePool.Allocate(texture.mDesc, { mFrameIndex, 0 });
        texture.m_pFrameTexture = allocated.first;
        texture.mInitState      = allocated.second;
    }
    return texture.m_pFrameTexture;
}

GPUBindTableID RenderGraphBackend::AllocateAndUpdatePassBindTable(RenderGraphFrameExecutor& executor, PassNode* pass, GPURootSignatureID root_sig)
{
    if (root_sig == nullptr) return nullptr;
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
    // SRV
    for (uint32_t i = 0; i < texReadEdges.size(); i++)
    {
        auto& readEdge = texReadEdges[i];
        assert(!readEdge->mName.empty());
        const auto& res = *FindShaderResource(readEdge->mNameHash, root_sig);
        bind_table_keys += readEdge->mName.empty() ? (const char*)res.name : readEdge->mName;
        bind_table_keys += ';';
        bindTableValueNames.emplace_back((const char*)res.name);

        auto texture_readed                = readEdge->GetTextureNode();
        GPUDescriptorData update           = {};
        update.count                       = 1;
        update.name                        = res.name;
        update.binding_type                = GPU_RESOURCE_TYPE_TEXTURE;
        update.binding                     = res.binding;
        GPUTextureViewDescriptor view_desc = {};
        view_desc.pTexture                 = Resolve(executor, *texture_readed);
        view_desc.baseArrayLayer           = readEdge->GetArrayBase();
        view_desc.arrayLayerCount          = readEdge->GetArrayCount();
        view_desc.baseMipLevel             = readEdge->GetMipBase();
        view_desc.mipLevelCount            = readEdge->GetMipCount();
        view_desc.format                   = (EGPUFormat)view_desc.pTexture->format;
        bool is_depth_stencil              = Utils::FormatUtil_IsDepthStencilFormat(view_desc.format);
        bool is_depth_only                 = Utils::FormatUtil_IsDepthOnlyFormat(view_desc.format);
        view_desc.aspectMask =
        is_depth_stencil ?
        is_depth_only ? GPU_TVA_DEPTH : GPU_TVA_DEPTH | GPU_TVA_STENCIL :
        GPU_TVA_COLOR;
        view_desc.usages = GPU_TVU_SRV;
        // view_desc.dims = read_edge->get_dimension();
        SRVs[i]         = mTextureViewPool.Allocate(view_desc, mFrameIndex);
        update.textures = &SRVs[i];
        desc_set_updates.emplace_back(update);
    }

    GPUBindTableID table = executor.mBindTablePools[root_sig]->Pop(bind_table_keys, bindTableValueNames.data(), (uint32_t)bindTableValueNames.size());
    GPUBindTableUpdate(table, desc_set_updates.data(), (uint32_t)desc_set_updates.size());
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

void RenderGraphBackend::DeallocaResources(PassNode* pass)
{
    pass->ForEachTextures([this, pass](TextureNode* texture, TextureEdge* edge)
    {
        if (texture->mImported) return;
        bool isLastUser = true;
        texture->ForeachNeighbors([&](DependencyGraphNode* node)
        {
            RenderGraphNode* renderNode = (RenderGraphNode*)node;
            if (renderNode->type == EObjectType::Pass)
            {
                isLastUser = isLastUser && pass->mOrder >= ((PassNode*)renderNode)->mOrder;
            }
        });
        if (isLastUser)
        {
            mTexturePool.Deallocate(texture->mDesc, texture->m_pFrameTexture, edge->mRequestedState, {mFrameIndex, 0});
        }
    });

    // for each buffer
}
//////////////////RenderGraphBackend////////////////////////