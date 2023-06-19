#include "api.h"
#include <memory>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "texture.h"

inline static void ReadBytes(const char8_t* file_name, uint32_t** bytes, uint32_t* length)
{
    FILE* f = fopen((const char*)file_name, "rb");
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (uint32_t*)malloc(*length);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

inline static void ReadShaderBytes(const char8_t* virtual_path, uint32_t** bytes, uint32_t* length, EGPUBackend backend)
{
    std::filesystem::path cur_path = std::filesystem::current_path();
    std::u8string shader_path      = cur_path.u8string();
    shader_path.append(u8"./../shaders/");
    char8_t shader_file[256];
    strcpy((char*)shader_file, (const char*)shader_path.c_str());
    strcat((char*)shader_file, (const char*)virtual_path);
    switch (backend)
    {
        case EGPUBackend::GPUBackend_Vulkan:
            strcat((char*)shader_file, (const char*)(u8".spv"));
            break;
        default:
            break;
    }
    ReadBytes(shader_file, bytes, length);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_DESTROY:
            std::cout << "\ndestroying window\n";
            PostQuitMessage(0);
            return 0L;
        case WM_LBUTTONDOWN:
            std::cout << "\nmouse left button down at (" << LOWORD(lp) << ',' << HIWORD(lp) << ")\n";
        default:
            return DefWindowProc(window, msg, wp, lp);
    }
}

HWND CreateWin32Window()
{
    // Register the window class.
    auto myclass        = L"myclass";
    WNDCLASSEX wndclass = {
        sizeof(WNDCLASSEX), CS_DBLCLKS,
        WindowProcedure,
        0, 0, GetModuleHandle(0), LoadIcon(0, IDI_APPLICATION),
        LoadCursor(0, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1),
        0, myclass, LoadIcon(0, IDI_APPLICATION)
    };
    static bool bRegistered = RegisterClassEx(&wndclass);
    if (bRegistered)
    {
        HWND window = CreateWindowEx(0, myclass, TEXT("title"),
                                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, GetModuleHandle(0), 0);
        if (window)
        {
            ShowWindow(window, SW_SHOWDEFAULT);
        }
        return window;
    }
    return nullptr;
}

GPUTextureID CreateTexture(GPUDeviceID device, GPUQueueID queue)
{
    GPUTextureDescriptor desc{};
    desc.flags = GPU_TCF_OWN_MEMORY_BIT;
    desc.width = TEXTURE_WIDTH;
    desc.height = TEXTURE_HEIGHT;
    desc.depth  = 1;
    desc.array_size = 1;
    desc.format     = GPU_FORMAT_R8G8BA8_UNORM;
    desc.owner_queue = queue;
    desc.start_state = GPU_RESOURCE_STATE_COPY_DEST;
    desc.descriptors = GPU_RESOURCE_TYPE_TEXTURE;
    return GPUCreateTexture(device, &desc);
}

GPUTextureViewID CreateTextureView(GPUTextureID texture)
{
    GPUTextureViewDescriptor desc{};
    desc.pTexture        = texture;
    desc.format          = (EGPUFormat)texture->format;
    desc.usage           = EGPUTexutreViewUsage::GPU_TVU_SRV;
    desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
    desc.baseMipLevel    = 0;
    desc.mipLevelCount   = 1;
    desc.baseArrayLayer  = 0;
    desc.arrayLayerCount = 1;
    return GPUCreateTextureView(texture->pDevice, &desc);
}

GPUSamplerID CreateTextureSampler(GPUDeviceID device)
{
    GPUSamplerDescriptor desc{};
    desc.min_filter = GPU_FILTER_TYPE_LINEAR;
    desc.mag_filter = GPU_FILTER_TYPE_LINEAR;
    desc.mipmap_mode = GPU_MIPMAP_MODE_LINEAR;
    desc.address_u   = GPU_ADDRESS_MODE_REPEAT;
    desc.address_v  = GPU_ADDRESS_MODE_REPEAT;
    desc.address_w   = GPU_ADDRESS_MODE_REPEAT;
    desc.compare_func = GPU_CMP_NEVER;
    return GPUCreateSampler(device, &desc);
}

int main(int argc, char** argv)
{
    GPUInstanceDescriptor desc{
        .pChained         = nullptr,
        .backend          = EGPUBackend::GPUBackend_Vulkan,
        .enableDebugLayer = true,
        .enableValidation = true
    };
    GPUInstanceID pInstance = GPUCreateInstance(&desc);
    uint32_t adapterCount   = 0;
    GPUEnumerateAdapters(pInstance, NULL, &adapterCount);
    DECLEAR_ZERO_VAL(GPUAdapterID, adapters, adapterCount);
    GPUEnumerateAdapters(pInstance, adapters, &adapterCount);

    auto window           = CreateWin32Window();
    GPUSurfaceID pSurface = GPUCreateSurfaceFromNativeView(pInstance, window);

    GPUQueueGroupDescriptor G = {
        .queueType  = EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS,
        .queueCount = 1
    };
    GPUDeviceDescriptor deviceDesc = {
        .pQueueGroup          = &G,
        .queueGroupCount      = 1,
        .disablePipelineCache = false
    };
    GPUDeviceID device = GPUCreateDevice(adapters[0], &deviceDesc);

    GPUQueueID pGraphicQueue = GPUGetQueue(device, EGPUQueueType::GPU_QUEUE_TYPE_GRAPHICS, 0);

    GPUFenceID presenFence = GPUCreateFence(device);

    GPUSwapchainDescriptor swapchainDesc{};
    swapchainDesc.ppPresentQueues    = &pGraphicQueue;
    swapchainDesc.presentQueuesCount = 1;
    swapchainDesc.pSurface           = pSurface;
    swapchainDesc.format             = EGPUFormat::GPU_FORMAT_B8G8R8A8_UNORM;
    swapchainDesc.width              = 1280;
    swapchainDesc.height             = 720;
    swapchainDesc.imageCount         = 3;
    swapchainDesc.enableVSync        = true;
    GPUSwapchainID pSwapchain        = GPUCreateSwapchain(device, &swapchainDesc);
    GPUTextureViewID ppSwapchainImage[3];
    for (uint32_t i = 0; i < 3; i++)
    {
        GPUTextureViewDescriptor desc{};
        desc.pTexture        = pSwapchain->ppBackBuffers[i];
        desc.format          = (EGPUFormat)desc.pTexture->format;
        desc.usage           = EGPUTexutreViewUsage::GPU_TVU_RTV_DSV;
        desc.aspectMask      = EGPUTextureViewAspect::GPU_TVA_COLOR;
        desc.baseMipLevel    = 0;
        desc.mipLevelCount   = 1;
        desc.baseArrayLayer  = 0;
        desc.arrayLayerCount = 1;

        ppSwapchainImage[i] = GPUCreateTextureView(device, &desc);
    }
    //shader
    uint32_t* vShaderCode;
    uint32_t vSize = 0;
    ReadShaderBytes(u8"vertex_shader.vert", &vShaderCode, &vSize, EGPUBackend::GPUBackend_Vulkan);
    uint32_t* fShaderCode;
    uint32_t fSize = 0;
    ReadShaderBytes(u8"fragment_shader.frag", &fShaderCode, &fSize, EGPUBackend::GPUBackend_Vulkan);
    GPUShaderLibraryDescriptor vShaderDesc{};
    vShaderDesc.pName    = u8"vertex_shader";
    vShaderDesc.code = vShaderCode;
    vShaderDesc.codeSize = vSize;
    vShaderDesc.stage    = GPU_SHADER_STAGE_VERT;
    GPUShaderLibraryDescriptor fShaderDesc{};
    fShaderDesc.pName    = u8"fragment_shader";
    fShaderDesc.code     = fShaderCode;
    fShaderDesc.codeSize = fSize;
    fShaderDesc.stage    = GPU_SHADER_STAGE_FRAG;
    GPUShaderLibraryID pVShader = GPUCreateShaderLibrary(device, &vShaderDesc);
    GPUShaderLibraryID pFShader = GPUCreateShaderLibrary(device, &fShaderDesc);
    free(vShaderCode);
    free(fShaderCode);

    GPUShaderEntryDescriptor shaderEntries[2] = {0};
    shaderEntries[0].stage                    = GPU_SHADER_STAGE_VERT;
    shaderEntries[0].entry                    = u8"main";
    shaderEntries[0].pLibrary                 = pVShader;
    shaderEntries[1].stage                    = GPU_SHADER_STAGE_FRAG;
    shaderEntries[1].entry                    = u8"main";
    shaderEntries[1].pLibrary                 = pFShader;

    GPUSamplerID texture_sampler              = CreateTextureSampler(device);
    GPUTextureID texture                      = CreateTexture(device, pGraphicQueue);
    GPUTextureViewID textureView              = CreateTextureView(texture);
    const char8_t* sampler_name               = u8"texSamp";
    GPURootSignatureDescriptor rootRSDesc     = {};
    rootRSDesc.shaders                        = shaderEntries;
    rootRSDesc.shader_count                   = 2;
    rootRSDesc.static_sampler_names           = &sampler_name;
    rootRSDesc.static_sampler_count           = 1;
    rootRSDesc.static_samplers                = &texture_sampler;
    GPURootSignatureID pRS                    = GPUCreateRootSignature(device, &rootRSDesc);
    GPUDescriptorSetDescriptor set_desc{};
    set_desc.root_signature = pRS;
    set_desc.set_index      = 0;
    GPUDescriptorSetID set  = GPUCreateDescriptorSet(device, &set_desc);
    //set_desc.set_index      = 1;
    //GPUDescriptorSetID set_1  = GPUCreateDescriptorSet(device, &set_desc);
    GPUVertexLayout vertexLayout{};
    vertexLayout.attributeCount = 3;
    vertexLayout.attributes[0]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, 0, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[1]  = { 1, GPU_FORMAT_R32G32B32_SFLOAT, 0, sizeof(float) * 3, sizeof(float) * 3, GPU_INPUT_RATE_VERTEX };
    vertexLayout.attributes[2]  = { 1, GPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 6, sizeof(float) * 2, GPU_INPUT_RATE_VERTEX };
    GPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.pRootSignature    = pRS;
    pipelineDesc.pVertexShader     = &shaderEntries[0];
    pipelineDesc.pFragmentShader   = &shaderEntries[1];
    pipelineDesc.pVertexLayout     = &vertexLayout;
    pipelineDesc.primitiveTopology = GPU_PRIM_TOPO_TRI_LIST;
    pipelineDesc.pColorFormats     = const_cast<EGPUFormat*>(&ppSwapchainImage[0]->desc.format);
    pipelineDesc.renderTargetCount = 1;
    GPURenderPipelineID pipeline   = GPUCreateRenderPipeline(device, &pipelineDesc);
    GPUFreeShaderLibrary(pVShader);
    GPUFreeShaderLibrary(pFShader);

    ////update descriptorset
    //GPUDescriptorData desc_data{};
    //desc_data.name         = u8"tex";//shader sampler2D`s name
    //desc_data.binding      = 0;
    //desc_data.binding_type = GPU_RESOURCE_TYPE_TEXTURE;
    //desc_data.textures     = &textureView;
    //desc_data.samplers     = &texture_sampler;
    ////GPUUpdateDescriptorSet(set, &desc_data, 1);

    //render loop begin
    GPUCommandPoolID pool = GPUCreateCommandPool(pGraphicQueue);
    GPUCommandBufferDescriptor cmdDesc{};
    cmdDesc.isSecondary = false;
    GPUCommandBufferID cmd = GPUCreateCommandBuffer(pool, &cmdDesc);
    uint32_t backbufferIndex = 0;

    struct Vertex {
        float x;
        float y;
        float z;
        float r;
        float g;
        float b;
        float u;
        float v;
    };
    Vertex vertices[] = {
        { 0.0f, -0.5f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f },
        { -0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f },
        { 0.5f, 0.5f, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f },
        { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }
    };
    /*Vertex vertices[] = {
        { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f },
        { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
        { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
    };*/
    GPUBufferDescriptor upload_buffer{};
    upload_buffer.size         = sizeof(TEXTURE_DATA);
    upload_buffer.flags        = GPU_BCF_OWN_MEMORY_BIT | GPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer.descriptors  = GPU_RESOURCE_TYPE_NONE;
    upload_buffer.memory_usage = GPU_MEM_USAGE_CPU_ONLY;
    GPUBufferID uploadBuffer   = GPUCreateBuffer(device, &upload_buffer);
    //copy texture
    memcpy(uploadBuffer->cpu_mapped_address, TEXTURE_DATA, sizeof(TEXTURE_DATA));
    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        GPUBufferToTextureTransfer trans_texture_buffer_desc{};
        trans_texture_buffer_desc.dst                              = texture;
        trans_texture_buffer_desc.dst_subresource.mip_level        = 0;
        trans_texture_buffer_desc.dst_subresource.base_array_layer = 0;
        trans_texture_buffer_desc.dst_subresource.layer_count      = 1;
        trans_texture_buffer_desc.src                              = uploadBuffer;
        trans_texture_buffer_desc.src_offset                       = 0;
        GPUCmdTransferBufferToTexture(cmd, &trans_texture_buffer_desc);
        GPUTextureBarrier barrier{};
        barrier.texture = texture;
        barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        barrier.dst_state = GPU_RESOURCE_STATE_SHADER_RESOURCE;
        GPUResourceBarrierDescriptor rs_barrer{};
        rs_barrer.texture_barriers      = &barrier;
        rs_barrer.texture_barriers_count = 1;
        GPUCmdResourceBarrier(cmd, &rs_barrer);
    }
    GPUCmdEnd(cmd);
    GPUQueueSubmitDescriptor texture_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
    GPUSubmitQueue(pGraphicQueue, &texture_cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);

    //vertex buffer
    GPUBufferDescriptor vertex_desc{};
    vertex_desc.size         = sizeof(vertices);
    vertex_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    vertex_desc.descriptors  = GPU_RESOURCE_TYPE_VERTEX_BUFFER;
    vertex_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    GPUBufferID vertexBuffer = GPUCreateBuffer(device, &vertex_desc);
    //COPY
    memcpy(uploadBuffer->cpu_mapped_address, vertices, sizeof(vertices));
    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        GPUBufferToBufferTransfer trans_verticex_buffer_desc{};
        trans_verticex_buffer_desc.size       = sizeof(vertices);
        trans_verticex_buffer_desc.src        = uploadBuffer;
        trans_verticex_buffer_desc.src_offset = 0;
        trans_verticex_buffer_desc.dst        = vertexBuffer;
        trans_verticex_buffer_desc.dst_offset = 0;
        GPUTransferBufferToBuffer(cmd, &trans_verticex_buffer_desc);
        GPUBufferBarrier vertex_barrier{};
        vertex_barrier.buffer    = vertexBuffer;
        vertex_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        vertex_barrier.dst_state = GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        GPUResourceBarrierDescriptor vertex_rs_barrer{};
        vertex_rs_barrer.buffer_barriers       = &vertex_barrier;
        vertex_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmd, &vertex_rs_barrer);
    }
    GPUCmdEnd(cmd);
    GPUQueueSubmitDescriptor cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
    GPUSubmitQueue(pGraphicQueue, &cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);

    //index buffer
    uint16_t indices[] = {
        0, 1, 2, 1, 3, 2
    };
    GPUBufferDescriptor index_desc{};
    index_desc.size         = sizeof(indices);
    index_desc.flags        = GPU_BCF_OWN_MEMORY_BIT;
    index_desc.descriptors  = GPU_RESOURCE_TYPE_INDEX_BUFFER;
    index_desc.memory_usage = GPU_MEM_USAGE_GPU_ONLY;
    GPUBufferID indexBuffer = GPUCreateBuffer(device, &index_desc);
    //copy
    memcpy(uploadBuffer->cpu_mapped_address, indices, sizeof(indices));
    GPUResetCommandPool(pool);
    GPUCmdBegin(cmd);
    {
        GPUBufferToBufferTransfer trans_index_buffer_desc{};
        trans_index_buffer_desc.size       = sizeof(indices);
        trans_index_buffer_desc.src        = uploadBuffer;
        trans_index_buffer_desc.src_offset = 0;
        trans_index_buffer_desc.dst        = indexBuffer;
        trans_index_buffer_desc.dst_offset = 0;
        GPUTransferBufferToBuffer(cmd, &trans_index_buffer_desc);
        GPUBufferBarrier index_barrier{};
        index_barrier.buffer    = indexBuffer;
        index_barrier.src_state = GPU_RESOURCE_STATE_COPY_DEST;
        index_barrier.dst_state = GPU_RESOURCE_STATE_INDEX_BUFFER;
        GPUResourceBarrierDescriptor index_rs_barrer{};
        index_rs_barrer.buffer_barriers        = &index_barrier;
        index_rs_barrer.buffer_barriers_count = 1;
        GPUCmdResourceBarrier(cmd, &index_rs_barrer);
    }
    GPUCmdEnd(cmd);
    GPUQueueSubmitDescriptor index_cpy_submit = { .cmds = &cmd, .cmds_count = 1 };
    GPUSubmitQueue(pGraphicQueue, &index_cpy_submit);
    GPUWaitQueueIdle(pGraphicQueue);

    GPUFreeBuffer(uploadBuffer);
    // update descriptorset
    GPUDescriptorData desc_data[2] = {};
    desc_data[0].name         = u8"tex"; // shader sampler2D`s name
    desc_data[0].binding      = 0;
    desc_data[0].binding_type = GPU_RESOURCE_TYPE_TEXTURE;
    desc_data[0].count             = 1;
    desc_data[0].textures     = &textureView;
    desc_data[1].name              = u8"texSamp";
    desc_data[1].samplers      = &texture_sampler;
    desc_data[1].count             = 1;
    GPUUpdateDescriptorSet(set, desc_data, 1);
    //GPUUpdateDescriptorSet(set_1, desc_data + 1, 1);
    bool exit = false;
    MSG msg{};
    while (!exit)
    {
        if (PeekMessage(&msg, nullptr, 0, 0,  PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                exit = true;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            GPUWaitFences(&presenFence, 1);

            GPUAcquireNextDescriptor acq_desc{};
            acq_desc.fence                   = presenFence;
            backbufferIndex                  = GPUAcquireNextImage(pSwapchain, &acq_desc);
            GPUTextureID backbuffer          = pSwapchain->ppBackBuffers[backbufferIndex];
            GPUTextureViewID backbuffer_view = ppSwapchainImage[backbufferIndex];

            GPUResetCommandPool(pool);
            GPUCmdBegin(cmd);
            {
                GPUTextureBarrier tex_barrier{};
                tex_barrier.texture   = backbuffer;
                tex_barrier.src_state = GPU_RESOURCE_STATE_UNDEFINED;
                tex_barrier.dst_state = GPU_RESOURCE_STATE_RENDER_TARGET;
                GPUResourceBarrierDescriptor draw_barrier{};
                draw_barrier.texture_barriers       = &tex_barrier;
                draw_barrier.texture_barriers_count = 1;
                GPUCmdResourceBarrier(cmd, &draw_barrier);

                GPUColorAttachment screenAttachment{};
                screenAttachment.view         = backbuffer_view;
                screenAttachment.load_action  = GPU_LOAD_ACTION_CLEAR;
                screenAttachment.store_action = GPU_STORE_ACTION_STORE;
                screenAttachment.clear_color  = { { 0.f, 0.f, 0.f, 0.f } };
                GPURenderPassDescriptor render_pass_desc{};
                render_pass_desc.sample_count        = GPU_SAMPLE_COUNT_1;
                render_pass_desc.color_attachments   = &screenAttachment;
                render_pass_desc.render_target_count = 1;
                GPURenderPassEncoderID encoder       = GPUCmdBeginRenderPass(cmd, &render_pass_desc);
                {
                    GPURenderEncoderSetViewport(encoder, 0.f, 0.f, (float)backbuffer->width,
                                                (float)backbuffer->height, 0.f, 1.f);
                    GPURenderEncoderSetScissor(encoder, 0, 0, backbuffer->width,
                                               backbuffer->height);
                    GPURenderEncoderBindPipeline(encoder, pipeline);
                    //bind vertexbuffer
                    uint32_t stride = sizeof(Vertex);
                    GPURenderEncoderBindVertexBuffers(encoder, 1, &vertexBuffer, &stride, nullptr);
                    GPURenderEncoderBindIndexBuffer(encoder, indexBuffer, 0, sizeof(uint16_t));
                    //bind descriptor ste
                    GPURenderEncoderBindDescriptorSet(encoder, set);
                    //GPURenderEncoderDraw(encoder, 3, 0);
                    GPURenderEncoderDrawIndexed(encoder, sizeof(indices) / sizeof(uint16_t), 0, 0);
                }
                GPUCmdEndRenderPass(cmd, encoder);

                GPUTextureBarrier tex_barrier1{};
                tex_barrier1.texture   = backbuffer;
                tex_barrier1.src_state = GPU_RESOURCE_STATE_RENDER_TARGET;
                tex_barrier1.dst_state = GPU_RESOURCE_STATE_PRESENT;
                GPUResourceBarrierDescriptor present_barrier{};
                present_barrier.texture_barriers_count = 1;
                present_barrier.texture_barriers       = &tex_barrier1;
                GPUCmdResourceBarrier(cmd, &present_barrier);
            }
            GPUCmdEnd(cmd);

            // submit
            GPUQueueSubmitDescriptor submitDesc{};
            submitDesc.cmds       = &cmd;
            submitDesc.cmds_count = 1;
            GPUSubmitQueue(pGraphicQueue, &submitDesc);
            // present
            GPUWaitQueueIdle(pGraphicQueue);
            GPUQueuePresentDescriptor presentDesc{};
            presentDesc.swapchain = pSwapchain;
            presentDesc.index     = backbufferIndex;
            GPUQueuePresent(pGraphicQueue, &presentDesc);
        }
    }
    //render loop end

    GPUWaitQueueIdle(pGraphicQueue);
    GPUWaitFences(&presenFence, 1);
    GPUFreeFence(presenFence);
    for (uint32_t i = 0; i < pSwapchain->backBuffersCount; i++)
    {
        GPUFreeTextureView(ppSwapchainImage[i]);
    }
    GPUFreeSampler(texture_sampler);
    GPUFreeDescriptorSet(set);
    //GPUFreeDescriptorSet(set_1);
    GPUFreeTextureView(textureView);
    GPUFreeTexture(texture);
    GPUFreeBuffer(vertexBuffer);
    GPUFreeBuffer(indexBuffer);
    GPUFreeCommandBuffer(cmd);
    GPUFreeCommandPool(pool);
    GPUFreeRenderPipeline(pipeline);
    GPUFreeRootSignature(pRS);
    GPUFreeSwapchain(pSwapchain);
    GPUFreeQueue(pGraphicQueue);
    GPUFreeDevice(device);
    GPUFreeSurface(pInstance, pSurface);
    GPUFreeInstance(pInstance);

    DestroyWindow(window);
    return 0;
}