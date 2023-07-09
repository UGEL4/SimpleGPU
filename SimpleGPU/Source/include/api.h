#pragma once
#include "gpuconfig.h"
#include <stdint.h>

#if defined(__cplusplus)
#if defined(_MSC_VER) && !defined(__clang__)
    #define DECLEAR_ZERO_VAL(type, var, num)              \
        type* var = (type*)_alloca(sizeof(type) * (num)); \
        memset((void*)var, 0, sizeof(type) * (num));
#else
    #define DECLEAR_ZERO_VAL(type, var, num) \
        type var[(num)];                    \
        memset((void*)var, 0, sizeof(type) * (num));
#endif
#endif

#define GPU_SAFE_FREE(ptr) \
    if (ptr) free(ptr);\
	ptr = NULL;

#define DEFINE_GPU_OBJECT(name) struct name##Descriptor; typedef const struct name* name##ID;

DEFINE_GPU_OBJECT(GPUSurface)
DEFINE_GPU_OBJECT(GPUInstance)
DEFINE_GPU_OBJECT(GPUAdapter)
DEFINE_GPU_OBJECT(GPUDevice)
DEFINE_GPU_OBJECT(GPUQueue)
DEFINE_GPU_OBJECT(GPUSwapchain)
DEFINE_GPU_OBJECT(GPUTexture)
DEFINE_GPU_OBJECT(GPUTextureView)
DEFINE_GPU_OBJECT(GPUShaderLibrary)
DEFINE_GPU_OBJECT(GPURootSignature)
DEFINE_GPU_OBJECT(GPURenderPipeline)
DEFINE_GPU_OBJECT(GPUSampler)
DEFINE_GPU_OBJECT(GPURootSignaturePool)
DEFINE_GPU_OBJECT(GPUCommandPool)
DEFINE_GPU_OBJECT(GPUCommandBuffer)
DEFINE_GPU_OBJECT(GPUFence)
DEFINE_GPU_OBJECT(GPUSemaphore)
DEFINE_GPU_OBJECT(GPUBuffer)
DEFINE_GPU_OBJECT(GPURenderPassEncoder)
DEFINE_GPU_OBJECT(GPUDescriptorSet)

#ifdef __cplusplus
extern "C" {
#endif

    #define GPU_MAX_VERTEX_ATTRIBS 15
    #define GPU_MAX_MRT_COUNT 8u

    #define GPU_COLOR_MASK_RED 0x1
    #define GPU_COLOR_MASK_GREEN 0x2
    #define GPU_COLOR_MASK_BLUE 0x4
    #define GPU_COLOR_MASK_ALPHA 0x8
    #define GPU_COLOR_MASK_ALL GPU_COLOR_MASK_RED | GPU_COLOR_MASK_GREEN | GPU_COLOR_MASK_BLUE | GPU_COLOR_MASK_ALPHA
    #define GPU_COLOR_MASK_NONE 0

	typedef enum EGPUBackend
	{
		GPUBackend_Vulkan = 0,
        GPUBackend_D3D12  = 1,
		GPUBackend_Count,
		GPUBackend_MAX_ENUM_BIT = 0x7FFFFFFF
	} EGPUBackend;

	typedef enum EGPUQueueType
	{
		GPU_QUEUE_TYPE_GRAPHICS = 0,
		GPU_QUEUE_TYPE_COMPUTE = 1,
		GPU_QUEUE_TYPE_TRANSFER = 2,
		GPU_QUEUE_TYPE_COUNT,
		GPU_QUEUE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
	} EGPUQueueType;

	typedef enum EGPUFormat
	{
        GPU_FORMAT_UNDEFINED,
		GPU_FORMAT_B8G8R8A8_UNORM,
		GPU_FORMAT_B8G8R8A8_SRGB,
		GPU_FORMAT_R8G8BA8_UNORM,
		GPU_FORMAT_R8G8B8A8_SRGB,
        GPU_FORMAT_R16_UINT,
        GPU_FORMAT_R32_UINT,
        GPU_FORMAT_R32_SFLOAT,
        GPU_FORMAT_R32G32_SFLOAT,
        GPU_FORMAT_R32G32B32_SFLOAT,
        GPU_FORMAT_D16_UNORM_S8_UINT,
        GPU_FORMAT_D24_UNORM_S8_UINT,
        GPU_FORMAT_D32_SFLOAT_S8_UINT,
        GPU_FORMAT_D16_UNORM,
        GPU_FORMAT_D32_SFLOAT,
		GPU_FORMAT_COUNT
	} EGPUFormat;

	typedef enum EGPUSampleCount
    {
        GPU_SAMPLE_COUNT_1 = 1,
        GPU_SAMPLE_COUNT_2 = 2,
        GPU_SAMPLE_COUNT_4 = 4,
        GPU_SAMPLE_COUNT_8 = 8,
        GPU_SAMPLE_COUNT_16 = 16,
        GPU_SAMPLE_COUNT_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUSampleCount;

    typedef enum EGPUTexutreViewUsage
    {
        GPU_TVU_SRV          = 0x01,
        GPU_TVU_RTV_DSV      = 0x02,
        GPU_TVU_UAV          = 0x04,
        GPU_TVU_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUTexutreViewUsage;

    typedef enum EGPUTextureViewAspect
    {
        GPU_TVA_COLOR        = 0x01,
        GPU_TVA_DEPTH        = 0x02,
        GPU_TVA_STENCIL      = 0x04,
        GPU_TVA_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUTextureViewAspect;
    typedef uint32_t GPUTextureViewAspects;

    // Same Value As Vulkan Enumeration Bits.
    typedef enum EGPUShaderStage
    {
        GPU_SHADER_STAGE_NONE = 0,

        GPU_SHADER_STAGE_VERT       = 0X00000001,
        GPU_SHADER_STAGE_TESC       = 0X00000002,
        GPU_SHADER_STAGE_TESE       = 0X00000004,
        GPU_SHADER_STAGE_GEOM       = 0X00000008,
        GPU_SHADER_STAGE_FRAG       = 0X00000010,
        GPU_SHADER_STAGE_COMPUTE    = 0X00000020,
        GPU_SHADER_STAGE_RAYTRACING = 0X00000040,
        GPU_SHADER_STAGE_ALL_GRAPHICS = (uint32_t)GPU_SHADER_STAGE_VERT | (uint32_t)GPU_SHADER_STAGE_TESC | (uint32_t)GPU_SHADER_STAGE_TESE | (uint32_t)GPU_SHADER_STAGE_GEOM | (uint32_t)GPU_SHADER_STAGE_FRAG,
        GPU_SHADER_STAGE_HULL         = GPU_SHADER_STAGE_TESC,
        GPU_SHADER_STAGE_DOMAIN       = GPU_SHADER_STAGE_TESE,
        GPU_SHADER_STAGE_COUNT        = 6,
        GPU_SHADER_STAGE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUShaderStage;
    typedef uint32_t GPUShaderStages;

    typedef enum EGPUVertexInputRate
    {
        GPU_INPUT_RATE_VERTEX   = 0,
        GPU_INPUT_RATE_INSTANCE = 1,
        GPU_INPUT_RATE_COUNT,
        GPU_INPUT_RATE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUVertexInputRate;

    typedef enum EGPUPrimitiveTopology
    {
        GPU_PRIM_TOPO_POINT_LIST = 0,
        GPU_PRIM_TOPO_LINE_LIST,
        GPU_PRIM_TOPO_LINE_STRIP,
        GPU_PRIM_TOPO_TRI_LIST,
        GPU_PRIM_TOPO_TRI_STRIP,
        GPU_PRIM_TOPO_PATCH_LIST,
        GPU_PRIM_TOPO_COUNT,
        GPU_PRIM_TOPO_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUPrimitiveTopology;

    typedef enum EGPUCompareMode
    {
        GPU_CMP_NEVER,
        GPU_CMP_LESS,
        GPU_CMP_EQUAL,
        GPU_CMP_LEQUAL,
        GPU_CMP_GREATER,
        GPU_CMP_NOTEQUAL,
        GPU_CMP_GEQUAL,
        GPU_CMP_ALWAYS,
        GPU_CMP_COUNT,
        GPU_CMP_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUCompareMode;

    typedef enum EGPUStencilOp
    {
        GPU_STENCIL_OP_KEEP,
        GPU_STENCIL_OP_SET_ZERO,
        GPU_STENCIL_OP_REPLACE,
        GPU_STENCIL_OP_INVERT,
        GPU_STENCIL_OP_INCR,
        GPU_STENCIL_OP_DECR,
        GPU_STENCIL_OP_INCR_SAT,
        GPU_STENCIL_OP_DECR_SAT,
        GPU_STENCIL_OP_COUNT,
        GPU_STENCIL_OP_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUStencilOp;

    typedef enum EGPUCullMode
    {
        GPU_CULL_MODE_NONE = 0,
        GPU_CULL_MODE_BACK,
        GPU_CULL_MODE_FRONT,
        GPU_CULL_MODE_BOTH,
        GPU_CULL_MODE_COUNT,
        GPU_CULL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUCullMode;

    typedef enum EGPUFrontFace
    {
        GPU_FRONT_FACE_CCW = 0,
        GPU_FRONT_FACE_CW,
        GPU_FRONT_FACE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUFrontFace;

    typedef enum EGPUFillMode
    {
        GPU_FILL_MODE_SOLID,
        GPU_FILL_MODE_WIREFRAME,
        GPU_FILL_MODE_COUNT,
        GPU_FILL_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUFillMode;

    typedef enum EGPUBlendConstant
    {
        GPU_BLEND_CONST_ZERO = 0,
        GPU_BLEND_CONST_ONE,
        GPU_BLEND_CONST_SRC_COLOR,
        GPU_BLEND_CONST_ONE_MINUS_SRC_COLOR,
        GPU_BLEND_CONST_DST_COLOR,
        GPU_BLEND_CONST_ONE_MINUS_DST_COLOR,
        GPU_BLEND_CONST_SRC_ALPHA,
        GPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA,
        GPU_BLEND_CONST_DST_ALPHA,
        GPU_BLEND_CONST_ONE_MINUS_DST_ALPHA,
        GPU_BLEND_CONST_SRC_ALPHA_SATURATE,
        GPU_BLEND_CONST_BLEND_FACTOR,
        GPU_BLEND_CONST_ONE_MINUS_BLEND_FACTOR,
        GPU_BLEND_CONST_COUNT,
        GPU_BLEND_CONST_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUBlendConstant;

    typedef enum EGPUBlendMode
    {
        GPU_BLEND_MODE_ADD,
        GPU_BLEND_MODE_SUBTRACT,
        GPU_BLEND_MODE_REVERSE_SUBTRACT,
        GPU_BLEND_MODE_MIN,
        GPU_BLEND_MODE_MAX,
        GPU_BLEND_MODE_COUNT,
        GPU_BLEND_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUBlendMode;

    typedef enum EGPULoadAction
    {
        GPU_LOAD_ACTION_DONTCARE,
        GPU_LOAD_ACTION_LOAD,
        GPU_LOAD_ACTION_CLEAR,
        GPU_LOAD_ACTION_COUNT,
        GPU_LOAD_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPULoadAction;

    typedef enum EGPUStoreAction
    {
        GPU_STORE_ACTION_STORE,
        GPU_STORE_ACTION_DISCARD,
        GPU_STORE_ACTION_COUNT,
        GPU_STORE_ACTION_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUStoreAction;

    typedef enum EGPUPipelineType
    {
        GPU_PIPELINE_TYPE_NONE = 0,
        GPU_PIPELINE_TYPE_COMPUTE,
        GPU_PIPELINE_TYPE_GRAPHICS,
        GPU_PIPELINE_TYPE_RAYTRACING,
        GPU_PIPELINE_TYPE_COUNT,
        GPU_PIPELINE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUPipelineType;

    typedef enum EGPUFenceStatus
    {
        GPU_FENCE_STATUS_COMPLETE = 0,
        GPU_FENCE_STATUS_INCOMPLETE,
        GPU_FENCE_STATUS_NOTSUBMITTED,
        GPU_FENCE_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUFenceStatus;

    typedef enum EGPUMemoryUsage
    {
        /// No intended memory usage specified.
        GPU_MEM_USAGE_UNKNOWN = 0,
        /// Memory will be used on device only, no need to be mapped on host.
        GPU_MEM_USAGE_GPU_ONLY = 1,
        /// Memory will be mapped on host. Could be used for transfer to device.
        GPU_MEM_USAGE_CPU_ONLY = 2,
        /// Memory will be used for frequent (dynamic) updates from host and reads on device.
        /// Memory location (heap) is unsure.
        GPU_MEM_USAGE_CPU_TO_GPU = 3,
        /// Memory will be used for writing on device and readback on host.
        /// Memory location (heap) is unsure.
        GPU_MEM_USAGE_GPU_TO_CPU = 4,
        GPU_MEM_USAGE_COUNT,
        GPU_MEM_USAGE_MAX_ENUM = 0x7FFFFFFF
    } EGPUMemoryUsage;

    typedef enum EGPUBufferCreationFlag
    {
        /// Default flag (Buffer will use aliased memory, buffer will not be cpu accessible until mapBuffer is called)
        GPU_BCF_NONE = 0,
        /// Buffer will allocate its own memory (COMMITTED resource)
        GPU_BCF_OWN_MEMORY_BIT = 0x02,
        /// Buffer will be persistently mapped
        GPU_BCF_PERSISTENT_MAP_BIT = 0x04,
        /// Use ESRAM to store this buffer
        GPU_BCF_ESRAM = 0x08,
        /// Flag to specify not to allocate descriptors for the resource
        GPU_BCF_NO_DESCRIPTOR_VIEW_CREATION = 0x10,
        /// Flag to specify to create GPUOnly buffer as Host visible
        GPU_BCF_HOST_VISIBLE = 0x20,
#ifdef GPU_USE_METAL
        /* ICB Flags */
        /// Inherit pipeline in ICB
        GPU_BCF_ICB_INHERIT_PIPELINE = 0x100,
        /// Inherit pipeline in ICB
        GPU_BCF_ICB_INHERIT_BUFFERS = 0x200,
#endif
        GPU_BCF_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUBufferCreationFlag;
    typedef uint32_t GPUBufferCreationFlags;

    typedef enum EGPUFilterType
    {
        GPU_FILTER_TYPE_NEAREST = 0,
        GPU_FILTER_TYPE_LINEAR,
        GPU_FILTER_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
    } CGPUFilterType;

    typedef enum EGPUAddressMode
    {
        GPU_ADDRESS_MODE_MIRROR,
        GPU_ADDRESS_MODE_REPEAT,
        GPU_ADDRESS_MODE_CLAMP_TO_EDGE,
        GPU_ADDRESS_MODE_CLAMP_TO_BORDER,
        GPU_ADDRESS_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUAddressMode;

    typedef enum EGPUMipMapMode
    {
        GPU_MIPMAP_MODE_NEAREST = 0,
        GPU_MIPMAP_MODE_LINEAR,
        GPU_MIPMAP_MODE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUMipMapMode;

    typedef enum EGPUResourceState
    {
        GPU_RESOURCE_STATE_UNDEFINED                  = 0,
        GPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER = 0x1,
        GPU_RESOURCE_STATE_INDEX_BUFFER               = 0x2,
        GPU_RESOURCE_STATE_RENDER_TARGET              = 0x4,
        GPU_RESOURCE_STATE_UNORDERED_ACCESS           = 0x8,
        GPU_RESOURCE_STATE_DEPTH_WRITE                = 0x10,
        GPU_RESOURCE_STATE_DEPTH_READ                 = 0x20,
        GPU_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE  = 0x40,
        GPU_RESOURCE_STATE_PIXEL_SHADER_RESOURCE      = 0x80,
        GPU_RESOURCE_STATE_SHADER_RESOURCE            = 0x40 | 0x80,
        GPU_RESOURCE_STATE_STREAM_OUT                 = 0x100,
        GPU_RESOURCE_STATE_INDIRECT_ARGUMENT          = 0x200,
        GPU_RESOURCE_STATE_COPY_DEST                  = 0x400,
        GPU_RESOURCE_STATE_COPY_SOURCE                = 0x800,
        GPU_RESOURCE_STATE_GENERIC_READ               = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
        GPU_RESOURCE_STATE_PRESENT                    = 0x1000,
        GPU_RESOURCE_STATE_COMMON                     = 0x2000,
        GPU_RESOURCE_STATE_ACCELERATION_STRUCTURE     = 0x4000,
        GPU_RESOURCE_STATE_SHADING_RATE_SOURCE        = 0x8000,
        GPU_RESOURCE_STATE_RESOLVE_DEST               = 0x10000,
        GPU_RESOURCE_STATE_MAX_ENUM_BIT               = 0x7FFFFFFF
    } EGPUResourceState;
    typedef uint32_t GPUResourceStates;

        typedef enum EGPUResourceType
    {
        GPU_RESOURCE_TYPE_NONE    = 0,
        GPU_RESOURCE_TYPE_SAMPLER = 0x00000001,
        // SRV Read only texture
        GPU_RESOURCE_TYPE_TEXTURE = (GPU_RESOURCE_TYPE_SAMPLER << 1),
        /// RTV Texture
        GPU_RESOURCE_TYPE_RENDER_TARGET = (GPU_RESOURCE_TYPE_TEXTURE << 1),
        /// DSV Texture
        GPU_RESOURCE_TYPE_DEPTH_STENCIL = (GPU_RESOURCE_TYPE_RENDER_TARGET << 1),
        /// UAV Texture
        GPU_RESOURCE_TYPE_RW_TEXTURE = (GPU_RESOURCE_TYPE_DEPTH_STENCIL << 1),
        // SRV Read only buffer
        GPU_RESOURCE_TYPE_BUFFER     = (GPU_RESOURCE_TYPE_RW_TEXTURE << 1),
        GPU_RESOURCE_TYPE_BUFFER_RAW = (GPU_RESOURCE_TYPE_BUFFER | (GPU_RESOURCE_TYPE_BUFFER << 1)),
        /// UAV Buffer
        GPU_RESOURCE_TYPE_RW_BUFFER     = (GPU_RESOURCE_TYPE_BUFFER << 2),
        GPU_RESOURCE_TYPE_RW_BUFFER_RAW = (GPU_RESOURCE_TYPE_RW_BUFFER | (GPU_RESOURCE_TYPE_RW_BUFFER << 1)),
        /// CBV Uniform buffer
        GPU_RESOURCE_TYPE_UNIFORM_BUFFER = (GPU_RESOURCE_TYPE_RW_BUFFER << 2),
        /// Push constant / Root constant
        GPU_RESOURCE_TYPE_PUSH_CONSTANT = (GPU_RESOURCE_TYPE_UNIFORM_BUFFER << 1),
        /// IA
        GPU_RESOURCE_TYPE_VERTEX_BUFFER   = (GPU_RESOURCE_TYPE_PUSH_CONSTANT << 1),
        GPU_RESOURCE_TYPE_INDEX_BUFFER    = (GPU_RESOURCE_TYPE_VERTEX_BUFFER << 1),
        GPU_RESOURCE_TYPE_INDIRECT_BUFFER = (GPU_RESOURCE_TYPE_INDEX_BUFFER << 1),
        /// Cubemap SRV
        GPU_RESOURCE_TYPE_TEXTURE_CUBE = (GPU_RESOURCE_TYPE_TEXTURE | (GPU_RESOURCE_TYPE_INDIRECT_BUFFER << 1)),
        /// RTV / DSV per mip slice
        GPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES = (GPU_RESOURCE_TYPE_INDIRECT_BUFFER << 2),
        /// RTV / DSV per array slice
        GPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES = (GPU_RESOURCE_TYPE_RENDER_TARGET_MIP_SLICES << 1),
        /// RTV / DSV per depth slice
        GPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES = (GPU_RESOURCE_TYPE_RENDER_TARGET_ARRAY_SLICES << 1),
        GPU_RESOURCE_TYPE_RAY_TRACING                = (GPU_RESOURCE_TYPE_RENDER_TARGET_DEPTH_SLICES << 1),
#if defined(GPU_USE_VULKAN)
        /// Subpass input (descriptor type only available in Vulkan)
        GPU_RESOURCE_TYPE_INPUT_ATTACHMENT       = (GPU_RESOURCE_TYPE_RAY_TRACING << 1),
        GPU_RESOURCE_TYPE_TEXEL_BUFFER           = (GPU_RESOURCE_TYPE_INPUT_ATTACHMENT << 1),
        GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER        = (GPU_RESOURCE_TYPE_TEXEL_BUFFER << 1),
        GPU_RESOURCE_TYPE_COMBINED_IMAGE_SAMPLER = (GPU_RESOURCE_TYPE_RW_TEXEL_BUFFER << 1),
#endif
        GPU_RESOURCE_TYPE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUResourceType;
    typedef uint32_t GPUResourceTypes;

	//instance api
	GPUInstanceID GPUCreateInstance(const struct GPUInstanceDescriptor* pDesc);
	typedef GPUInstanceID (*GPUProcCreateInstance)(const struct GPUInstanceDescriptor* pDesc);
	void GPUFreeInstance(GPUInstanceID instance);
	typedef void (*GPUProcFreeInstance)(GPUInstanceID instance);

	//adapter api
    void GPUEnumerateAdapters(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount);
    typedef void (*GPUProcEnumerateAdapters)(GPUInstanceID pInstance, GPUAdapterID* const ppAdapters, uint32_t* adapterCount);

	//device api
    GPUDeviceID GPUCreateDevice(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc);
    typedef GPUDeviceID (*GPUProcCreateDevice)(GPUAdapterID pAdapter, const GPUDeviceDescriptor* pDesc);
    void GPUFreeDevice(GPUDeviceID pDevice);
    typedef void (*GPUProcFreeDevice)(GPUDeviceID pDevice);

    //queue
    GPUQueueID GPUGetQueue(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);
    typedef GPUQueueID (*GPUProcGetQueue)(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);
    void GPUFreeQueue(GPUQueueID queue);
    typedef void (*GPUProcFreeQueue)(GPUQueueID queue);
    void GPUSubmitQueue(GPUQueueID queue, const struct GPUQueueSubmitDescriptor* desc);
    typedef void (*GPUProcSubmitQueue)(GPUQueueID queue, const struct GPUQueueSubmitDescriptor* desc);
    void GPUWaitQueueIdle(GPUQueueID queue);
    typedef void (*GPUProcWaitQueueIdle)(GPUQueueID queue);
    void GPUQueuePresent(GPUQueueID queue, const struct GPUQueuePresentDescriptor* desc);
    typedef void (*GPUProcQueuePresent)(GPUQueueID queue, const struct GPUQueuePresentDescriptor* desc);


	// surface api
    GPUSurfaceID GPUCreateSurfaceFromNativeView(GPUInstanceID pInstance, void* view);
    void GPUFreeSurface(GPUInstanceID pInstance, GPUSurfaceID pSurface);
    typedef void (*GPUProcFreeSurface)(GPUInstanceID pInstance, GPUSurfaceID pSurface);
#if defined(_WIN64)
    typedef struct HWND__* HWND;
    GPUSurfaceID GPUCreateSurfaceFromHWND(GPUInstanceID pInstance, HWND window);
    typedef GPUSurfaceID (*GPUProcCreateSurfaceFromHWND)(GPUInstanceID pInstance, HWND window);
#endif

	//swapchain api
    GPUSwapchainID GPUCreateSwapchain(GPUDeviceID pDevice, GPUSwapchainDescriptor* pDesc);
    typedef GPUSwapchainID (*GPUProcCreateSwapchain)(GPUDeviceID pDevice, GPUSwapchainDescriptor* pDesc);
    void GPUFreeSwapchain(GPUSwapchainID pSwapchain);
    typedef void (*GPUProcFreeSwapchain)(GPUSwapchainID pSwapchain);
    uint32_t GPUAcquireNextImage(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor* desc);
    typedef uint32_t (*GPUProcAcquireNextImage)(GPUSwapchainID swapchain, const struct GPUAcquireNextDescriptor* desc);

	typedef struct GPUSurfacesProcTable
	{
        GPUProcFreeSurface FreeSurface;
#if defined(_WIN64)
        const GPUProcCreateSurfaceFromHWND CreateSurfaceFromHWND;
#endif
	} GPUSurfacesProcTable;

    //texture & texture_view api
    GPUTextureViewID GPUCreateTextureView(GPUDeviceID pDevice, const struct GPUTextureViewDescriptor* pDesc);
    typedef GPUTextureViewID (*GPUProcCreateTextureView)(GPUDeviceID pDevice, const struct GPUTextureViewDescriptor* pDesc);
    void GPUFreeTextureView(GPUTextureViewID pTextureView);
    typedef void (*GPUProcFreeTextureView)(GPUTextureViewID pTextureView);
    GPUTextureID GPUCreateTexture(GPUDeviceID device, const GPUTextureDescriptor* desc);
    typedef GPUTextureID (*GPUProcCreateTexture)(GPUDeviceID device, const GPUTextureDescriptor* desc);
    void GPUFreeTexture(GPUTextureID texture);
    typedef void (*GPUProcFreeTexture)(GPUTextureID texture);

    //shader api
    GPUShaderLibraryID GPUCreateShaderLibrary(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc);
    typedef GPUShaderLibraryID (*GPUProcCreateShaderLibrary)(GPUDeviceID pDevice, const GPUShaderLibraryDescriptor* pDesc);
    void GPUFreeShaderLibrary(GPUShaderLibraryID pShader);
    typedef void (*GPUProcFreeShaderLibrary)(GPUShaderLibraryID pShader);

    // pipeline
    GPURenderPipelineID GPUCreateRenderPipeline(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc);
    typedef GPURenderPipelineID (*GPUProcCreateRenderPipeline)(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc);
    void GPUFreeRenderPipeline(GPURenderPipelineID pPipeline);
    typedef void (*GPUProcFreeRenderPipeline)(GPURenderPipelineID pPipeline);

    GPURootSignatureID GPUCreateRootSignature(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc);
    typedef GPURootSignatureID (*GPUProcCreateRootSignature)(GPUDeviceID device, const struct GPURootSignatureDescriptor* desc);
    void GPUFreeRootSignature(GPURootSignatureID RS);
    typedef void (*GPUProcFreeRootSignature)(GPURootSignatureID RS);

    //command
    GPUCommandPoolID GPUCreateCommandPool(GPUQueueID queue);
    typedef GPUCommandPoolID (*GPUProcCreateCommandPool)(GPUQueueID queue);
    void GPUFreeCommandPool(GPUCommandPoolID pool);
    typedef void (*GPUProcFreeCommandPool)(GPUCommandPoolID pool);
    void GPUResetCommandPool(GPUCommandPoolID pool);
    typedef void (*GPUProcResetCommandPool)(GPUCommandPoolID pool);
    GPUCommandBufferID GPUCreateCommandBuffer(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc);
    typedef GPUCommandBufferID (*GPUProcCreateCommandBuffer)(GPUCommandPoolID pool, const GPUCommandBufferDescriptor* desc);
    void GPUFreeCommandBuffer(GPUCommandBufferID cmd);
    typedef void (*GPUProcFreeCommandBuffer)(GPUCommandBufferID cmd);
    void GPUCmdBegin(GPUCommandBufferID cmdBuffer);
    typedef void (*GPUProcCmdBegin)(GPUCommandBufferID cmdBuffer);
    void GPUCmdEnd(GPUCommandBufferID cmdBuffer);
    typedef void (*GPUProcCmdEnd)(GPUCommandBufferID cmdBuffer);
    void GPUCmdResourceBarrier(GPUCommandBufferID cmd, const struct GPUResourceBarrierDescriptor* desc);
    typedef void (*GPUProcCmdResourceBarrier)(GPUCommandBufferID cmd, const struct GPUResourceBarrierDescriptor* desc);
    void GPUCmdTransferBufferToTexture(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer* desc);
    typedef void (*GPUProcCmdTransferBufferToTexture)(GPUCommandBufferID cmd, const struct GPUBufferToTextureTransfer* desc);
    void GPUCmdTransferBufferToBuffer(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc);
    typedef void (*GPUProcCmdTransferBufferToBuffer)(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc);
    void GPUCmdTransferTextureToTexture(GPUCommandBufferID cmd, const struct GPUTextureToTextureTransfer* desc);
    typedef void (*GPUProcCmdTransferTextureToTexture)(GPUCommandBufferID cmd, const struct GPUTextureToTextureTransfer* desc);

    //fence & semaphore
    GPUFenceID GPUCreateFence(GPUDeviceID device);
    typedef GPUFenceID (*GPUProcCreateFence)(GPUDeviceID device);
    void GPUFreeFence(GPUFenceID fence);
    typedef void (*GPUProcFreeFence)(GPUFenceID fence);
    void GPUWaitFences(const GPUFenceID* fences, uint32_t fenceCount);
    typedef void (*GPUProcWaitFences)(const GPUFenceID* fences, uint32_t fenceCount);
    EGPUFenceStatus GPUQueryFenceStatus(GPUFenceID fence);
    typedef EGPUFenceStatus (*GPUProcQueryFenceStatus)(GPUFenceID fence);
    GPUSemaphoreID GPUCreateSemaphore(GPUDeviceID device);
    typedef GPUSemaphoreID (*GPUProcCreateSemaphore)(GPUDeviceID device);
    void GPUFreeSemaphore(GPUSemaphoreID semaphore);
    typedef void (*GPUProcFreeSemaphore)(GPUSemaphoreID semaphore);

    GPURenderPassEncoderID GPUCmdBeginRenderPass(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor* desc);
    typedef GPURenderPassEncoderID (*GPUProcCmdBeginRenderPass)(GPUCommandBufferID cmd, const struct GPURenderPassDescriptor* desc);
    void GPUCmdEndRenderPass(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder);
    typedef void (*GPUProcCmdEndRenderPass)(GPUCommandBufferID cmd, GPURenderPassEncoderID encoder);
    void GPURenderEncoderSetViewport(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth);
    typedef void (*GPUProcRenderEncoderSetViewport)(GPURenderPassEncoderID encoder, float x, float y, float width, float height, float min_depth, float max_depth);
    void GPURenderEncoderSetScissor(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    typedef void (*GPUProcRenderEncoderSetScissor)(GPURenderPassEncoderID encoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void GPURenderEncoderBindPipeline(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline);
    typedef void (*GPUProcRenderEncoderBindPipeline)(GPURenderPassEncoderID encoder, GPURenderPipelineID pipeline);
    void GPURenderEncoderDraw(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex);
    typedef void (*GPUProcRenderEncoderDraw)(GPURenderPassEncoderID encoder, uint32_t vertex_count, uint32_t first_vertex);
    void GPURenderEncoderDrawIndexed(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);
    typedef void (*GPUProcRenderEncoderDrawIndexed)(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);
    void GPURenderEncoderDrawIndexedInstanced(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    typedef void (*GPUProcRenderEncoderDrawIndexedInstanced)(GPURenderPassEncoderID encoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
    void GPURenderEncoderBindVertexBuffers(GPURenderPassEncoderID encoder, uint32_t buffer_count,
                                                  const GPUBufferID* buffers, const uint32_t* strides, const uint32_t* offsets);
    typedef void (*GPUProcRenderEncoderBindVertexBuffers)(GPURenderPassEncoderID encoder, uint32_t buffer_count,
                                                   const GPUBufferID* buffers, const uint32_t* strides, const uint32_t* offsets);
    void GPURenderEncoderBindIndexBuffer(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride);
    typedef void (*GPUProcRenderEncoderBindIndexBuffer)(GPURenderPassEncoderID encoder, GPUBufferID buffer, uint32_t offset, uint64_t indexStride);
    void GPURenderEncoderBindDescriptorSet(GPURenderPassEncoderID encoder, GPUDescriptorSetID set);
    typedef void (*GPUProcRenderEncoderBindDescriptorSet)(GPURenderPassEncoderID encoder, GPUDescriptorSetID set);

    //buffer
    GPUBufferID GPUCreateBuffer(GPUDeviceID device, const GPUBufferDescriptor* desc);
    typedef GPUBufferID (*GPUProcCreateBuffer)(GPUDeviceID device, const GPUBufferDescriptor* desc);
    void GPUFreeBuffer(GPUBufferID buffer);
    typedef void (*GPUProcFreeBuffer)(GPUBufferID buffer);
    void GPUTransferBufferToBuffer(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc);
    typedef void (*GPUProcTransferBufferToBuffer)(GPUCommandBufferID cmd, const struct GPUBufferToBufferTransfer* desc);

    //sampler
    GPUSamplerID GPUCreateSampler(GPUDeviceID device, const struct GPUSamplerDescriptor* desc);
    typedef GPUSamplerID (*GPUProcCreateSampler)(GPUDeviceID device, const struct GPUSamplerDescriptor* desc);
    void GPUFreeSampler(GPUSamplerID sampler);
    typedef void (*GPUProcFreeSampler)(GPUSamplerID sampler);

    GPUDescriptorSetID GPUCreateDescriptorSet(GPUDeviceID device, const struct GPUDescriptorSetDescriptor* desc);
    typedef GPUDescriptorSetID (*GPUProcCreateDescriptorSet)(GPUDeviceID device, const struct GPUDescriptorSetDescriptor* desc);
    void GPUFreeDescriptorSet(GPUDescriptorSetID set);
    typedef void (*GPUProcFreeDescriptorSet)(GPUDescriptorSetID set);
    void GPUUpdateDescriptorSet(GPUDescriptorSetID set, const struct GPUDescriptorData* datas, uint32_t count);
    typedef void (*GPUProcUpdateDescriptorSet)(GPUDescriptorSetID set, const struct GPUDescriptorData* datas, uint32_t count);

	typedef struct GPUProcTable
	{
		//instance api
        const GPUProcCreateInstance CreateInstance;
        const GPUProcFreeInstance FreeInstance;

        // adapter api
        const GPUProcEnumerateAdapters EnumerateAdapters;

        // device api
        const GPUProcCreateDevice CreateDevice;
        const GPUProcFreeDevice FreeDevice;

        //queue
        const GPUProcGetQueue GetQueue;
        const GPUProcFreeQueue FreeQueue;
        const GPUProcSubmitQueue SubmitQueue;
        const GPUProcWaitQueueIdle WaitQueueIdle;
        const GPUProcQueuePresent QueuePresent;

        // swapchain api
        const GPUProcCreateSwapchain CreateSwapchain;
        const GPUProcFreeSwapchain FreeSwapchain;
        const GPUProcAcquireNextImage AcquireNextImage;

        // texture & texture_view api
        const GPUProcCreateTextureView CreateTextureView;
        const GPUProcFreeTextureView FreeTextureView;
        const GPUProcCreateTexture CreateTexture;
        const GPUProcFreeTexture FreeTexture;

        //shader
        const GPUProcCreateShaderLibrary CreateShaderLibrary;
        const GPUProcFreeShaderLibrary FreeShaderLibrary;

        //pipeline
        const GPUProcCreateRenderPipeline CreateRenderPipeline;
        const GPUProcFreeRenderPipeline FreeRenderPipeline;

        const GPUProcCreateRootSignature CreateRootSignature;
        const GPUProcFreeRootSignature FreeRootSignature;

        //command
        const GPUProcCreateCommandPool CreateCommandPool;
        const GPUProcFreeCommandPool FreeCommandPool;
        const GPUProcResetCommandPool ResetCommandPool;
        const GPUProcCreateCommandBuffer CreateCommandBuffer;
        const GPUProcFreeCommandBuffer FreeCommandBuffer;
        const GPUProcCmdBegin CmdBegin;
        const GPUProcCmdEnd CmdEnd;
        const GPUProcCmdResourceBarrier CmdResourceBarrier;
        const GPUProcCmdTransferBufferToTexture CmdTransferBufferToTexture;
        const GPUProcCmdTransferBufferToBuffer CmdTransferBufferToBuffer;
        const GPUProcCmdTransferTextureToTexture CmdTransferTextureToTexture;

        //fence & semaphore
        const GPUProcCreateFence CreateFence;
        const GPUProcFreeFence FreeFence;
        const GPUProcWaitFences WaitFences;
        const GPUProcQueryFenceStatus QueryFenceStatus;
        const GPUProcCreateSemaphore GpuCreateSemaphore;
        const GPUProcFreeSemaphore GpuFreeSemaphore;

        const GPUProcCmdBeginRenderPass CmdBeginRenderPass;
        const GPUProcCmdEndRenderPass CmdEndRenderPass;
        const GPUProcRenderEncoderSetViewport RenderEncoderSetViewport;
        const GPUProcRenderEncoderSetScissor RenderEncoderSetScissor;
        const GPUProcRenderEncoderBindPipeline RenderEncoderBindPipeline;
        const GPUProcRenderEncoderDraw RenderEncoderDraw;
        const GPUProcRenderEncoderDrawIndexed RenderEncoderDrawIndexed;
        const GPUProcRenderEncoderDrawIndexedInstanced RenderEncoderDrawIndexedInstanced;
        const GPUProcRenderEncoderBindVertexBuffers RenderEncoderBindVertexBuffers;
        const GPUProcRenderEncoderBindIndexBuffer RenderEncoderBindIndexBuffer;
        const GPUProcRenderEncoderBindDescriptorSet RenderEncoderBindDescriptorSet;

        //buffer
        const GPUProcCreateBuffer CreateBuffer;
        const GPUProcFreeBuffer FreeBuffer;
        const GPUProcTransferBufferToBuffer TransferBufferToBuffer;

        //sampler
        const GPUProcCreateSampler CreateSampler;
        const GPUProcFreeSampler FreeSampler;

        const GPUProcCreateDescriptorSet CreateDescriptorSet;
        const GPUProcFreeDescriptorSet FreeDescriptorSet;
        const GPUProcUpdateDescriptorSet UpdateDescriptorSet;
	}GPUProcTable;

	typedef struct CGPUChainedDescriptor {
		EGPUBackend backend;
	} CGPUChainedDescriptor;

	typedef struct GPUInstanceDescriptor
	{
		const CGPUChainedDescriptor* pChained;
		EGPUBackend backend;
		bool enableDebugLayer;
		bool enableValidation;
	} GPUInstanceDescriptor;

	typedef struct GPUInstance
	{
		const GPUProcTable* pProcTable;
        const GPUSurfacesProcTable* pSurfaceProcTable;

		EGPUBackend backend;
	} GPUInstance;

    typedef struct GPUFormatSupport
    {
        uint8_t shader_read : 1;
        uint8_t shader_write : 1;
        uint8_t render_target_write : 1;
    } GPUFormatSupport;

	typedef struct GPUAdapterDetail
	{
        GPUFormatSupport format_supports[GPU_FORMAT_COUNT];
	} GPUAdapterDetail;

	typedef struct GPUAdapter
	{
        GPUInstanceID pInstance;
        const GPUProcTable* pProcTableCache;
	} GPUAdapter;

	typedef struct GPUQueueGroupDescriptor
	{
        EGPUQueueType queueType;
        uint32_t queueCount;
	} GPUQueueGroupDescriptor;

	typedef struct GPUDeviceDescriptor
	{
        GPUQueueGroupDescriptor* pQueueGroup;
        uint32_t queueGroupCount;
        bool disablePipelineCache;
	} GPUDeviceDescriptor;

	typedef struct GPUDevice
	{
        const GPUAdapterID pAdapter;
        const GPUProcTable* pProcTableCache;
        uint64_t nextTextureId;
	} GPUDevice;

	typedef struct GPUQueue
	{
        GPUDeviceID pDevice;
        EGPUQueueType queueType;
        uint32_t queueIndex;
	} GPUQueue;

	typedef struct GPUSwapchainDescriptor
	{
        GPUQueueID* ppPresentQueues;
        uint32_t presentQueuesCount;
        GPUSurfaceID pSurface;
        EGPUFormat format;
        uint32_t width;
        uint32_t height;
        uint32_t imageCount;
        bool enableVSync;
	} GPUSwapchainDescriptor;

	typedef struct GPUSwapchain
	{
        GPUDeviceID pDevice;
        const GPUTextureID* ppBackBuffers;
        uint32_t backBuffersCount;
	} GPUSwapchain;

    typedef enum EGPUTextureDimension
    {
        GPU_TEX_DIMENSION_1D,
        GPU_TEX_DIMENSION_2D,
        GPU_TEX_DIMENSION_2DMS,
        GPU_TEX_DIMENSION_3D,
        GPU_TEX_DIMENSION_CUBE,
        GPU_TEX_DIMENSION_1D_ARRAY,
        GPU_TEX_DIMENSION_2D_ARRAY,
        GPU_TEX_DIMENSION_2DMS_ARRAY,
        GPU_TEX_DIMENSION_CUBE_ARRAY,
        GPU_TEX_DIMENSION_COUNT,
        GPU_TEX_DIMENSION_UNDEFINED,
        GPU_TEX_DIMENSION_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUTextureDimension;

    typedef enum EGPUTextureCreationFlag
    {
        /// Default flag (Texture will use default allocation strategy decided by the api specific allocator)
        GPU_TCF_NONE = 0,
        /// Texture will allocate its own memory (COMMITTED resource)
        /// Note that this flag is not restricted Commited/Dedicated Allocation
        /// Actually VMA/D3D12MA allocate dedicated memories with ALLOW_ALIAS flag with specific loacl heaps
        /// If the texture needs to be restricted Committed/Dedicated(thus you want to keep its priority high)
        /// Toogle is_dedicated flag in GPUTextureDescriptor
        GPU_TCF_OWN_MEMORY_BIT = 0x01,
        /// Texture will be allocated in memory which can be shared among multiple processes
        GPU_TCF_EXPORT_BIT = 0x02,
        /// Texture will be allocated in memory which can be shared among multiple gpus
        GPU_TCF_EXPORT_ADAPTER_BIT = 0x04,
        /// Use on-tile memory to store this texture
        GPU_TCF_ON_TILE = 0x08,
        /// Prevent compression meta data from generating (XBox)
        GPU_TCF_NO_COMPRESSION = 0x10,
        /// Force 2D instead of automatically determining dimension based on width, height, depth
        GPU_TCF_FORCE_2D = 0x20,
        /// Force 3D instead of automatically determining dimension based on width, height, depth
        GPU_TCF_FORCE_3D = 0x40,
        /// Display target
        GPU_TCF_ALLOW_DISPLAY_TARGET = 0x80,
        /// Create a normal map texture
        GPU_TCF_NORMAL_MAP = 0x100,
        /// Fragment mask
        GPU_TCF_FRAG_MASK = 0x200,
        ///
        GPU_TCF_USABLE_MAX   = 0x40000,
        GPU_TCF_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUTextureCreationFlag;
    typedef uint32_t GPUTextureCreationFlags;

    typedef union GPUClearValue
    {
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
        struct
        {
            float depth;
            uint32_t stencil;
        };
    } GPUClearValue;

    typedef struct GPUTextureDescriptor
    {
        /// Texture creation flags (decides memory allocation strategy, sharing access,...)
        GPUTextureCreationFlags flags;
        /// Optimized clear value (recommended to use this same value when clearing the rendertarget)
        GPUClearValue clear_value;
        /// Width
        uint32_t width;
        /// Height
        uint32_t height;
        /// Depth (Should be 1 if not a mType is not TEXTURE_TYPE_3D)
        uint32_t depth;
        /// Texture array size (Should be 1 if texture is not a texture array or cubemap)
        uint32_t array_size;
        ///  image format
        EGPUFormat format;
        /// Number of mip levels
        uint32_t mip_levels;
        /// Number of multisamples per pixel (currently Textures created with mUsage TEXTURE_USAGE_SAMPLED_IMAGE only support CGPU_SAMPLE_COUNT_1)
        EGPUSampleCount sample_count;
        /// The image quality level. The higher the quality, the lower the performance. The valid range is between zero and the value appropriate for mSampleCount
        uint32_t sample_quality;
        /// Owner queue of the resource at creation
        GPUQueueID owner_queue;
        /// What state will the texture get created in
        EGPUResourceState start_state;
        /// Descriptor creation
        GPUResourceTypes descriptors;
        /// Memory Aliasing
        uint32_t is_dedicated;
        uint32_t is_aliasing;
    } GPUTextureDescriptor;

	typedef struct GPUTexture
	{
        GPUDeviceID pDevice;
        uint64_t sizeInBytes;
        EGPUSampleCount sampleCount : 8;
        /// Current state of the buffer
        uint32_t width : 24;
        uint32_t height : 24;
        uint32_t depth : 12;
        uint32_t mipLevels : 6;
        uint32_t arraySizeMinusOne : 12;
        uint32_t format : 12;
        /// Flags specifying which aspects (COLOR,DEPTH,STENCIL) are included in the pVkImageView
        uint32_t aspectMask : 4;
        uint32_t nodeIndex : 4;
        uint32_t isCube : 1;
        uint32_t isDedicated : 1;
        /// This value will be false if the underlying resource is not owned by the texture (swapchain textures,...)
        uint32_t ownsImage : 1;
        /// In CGPU concept aliasing resource owns no memory
        uint32_t isAliasing : 1;
        uint32_t canAlias : 1;
        uint32_t isImported : 1;
        uint32_t canExport : 1;
        void* nativeHandle;
        uint64_t uniqueId;
    } GPUTexture;

    typedef struct GPUTextureViewDescriptor
    {
        GPUTextureID pTexture;
        EGPUFormat format;
        EGPUTextureDimension dims;
        uint32_t usages;
        uint32_t aspectMask;
        uint32_t baseMipLevel;
        uint32_t mipLevelCount;
        uint32_t baseArrayLayer;
        uint32_t arrayLayerCount;
    } GPUTextureViewDescriptor;

    typedef struct GPUTextureView
    {
        GPUDeviceID pDevice;
        GPUTextureViewDescriptor desc;
    } GPUTextureView;

    typedef struct GPUTextureSubresource
    {
        GPUTextureViewAspects aspects;
        uint32_t mip_level;
        uint32_t base_array_layer;
        uint32_t layer_count;
    } GPUTextureSubresource;

    typedef struct GPUBufferToTextureTransfer
    {
        GPUTextureID dst;
        GPUTextureSubresource dst_subresource;
        GPUBufferID src;
        uint64_t src_offset;
    } GPUBufferToTextureTransfer;

    typedef struct GPUTextureToTextureTransfer
    {
        GPUTextureID src;
        GPUTextureID dst;
        GPUTextureSubresource src_subresource;
        GPUTextureSubresource dst_subresource;
    } GPUTextureToTextureTransfer;

    typedef struct GPUShaderLibraryDescriptor
    {
        const char8_t* pName;
        const uint32_t* code;
        uint32_t codeSize;
        EGPUShaderStage stage;
        bool reflectionOnly;
    } GPUShaderLibraryDescriptor;

    // Shaders
    typedef struct GPUShaderResource
    {
        const char8_t* name;
        uint64_t name_hash;
        EGPUResourceType type;
        EGPUTextureDimension dim;
        uint32_t set;
        uint32_t binding;
        uint32_t size;
        uint32_t offset;
        GPUShaderStages stages;
    } CGPUShaderResource;

    typedef struct GPUVertexInput
    {
        const char8_t* name;
        const char8_t* semantics;
        EGPUFormat format;
    } GPUVertexInput;

    typedef struct GPUShaderReflection
    {
        const char8_t* entry_name;
        EGPUShaderStage stage;
        GPUVertexInput* vertex_inputs;
        GPUShaderResource* shader_resources;
        uint32_t vertex_inputs_count;
        uint32_t shader_resources_count;
        uint32_t thread_group_sizes[3];
    } GPUShaderReflection;

    typedef struct GPUShaderLibrary
    {
        GPUDeviceID pDevice;
        char8_t* name;
        GPUShaderReflection* entry_reflections;
        uint32_t entrys_count;
    } CGPUShaderLibrary;

    typedef struct GPUShaderEntryDescriptor
    {
        GPUShaderLibraryID pLibrary;
        const char8_t* entry;
        EGPUShaderStage stage;
        // ++ constant_specialization
        //const CGPUConstantSpecialization* constants;
        //uint32_t num_constants;
        // -- constant_specialization
    } CGPUShaderEntryDescriptor;

    typedef struct GPUVertexAttribute
    {
        uint32_t arraySize;
        EGPUFormat format;
        uint32_t binding;
        uint32_t offset;
        uint32_t stride;
        EGPUVertexInputRate rate;
    } GPUVertexAttribute;

    typedef struct GPUVertexLayout
    {
        uint32_t attributeCount;
        GPUVertexAttribute attributes[GPU_MAX_VERTEX_ATTRIBS];
    } GPUVertexLayout;

    typedef struct GPURootSignatureDescriptor
    {
        struct GPUShaderEntryDescriptor* shaders;
        uint32_t shader_count;
        const GPUSamplerID* static_samplers;
        const char8_t* const* static_sampler_names;
        uint32_t static_sampler_count;
        const char8_t* const* push_constant_names;
        uint32_t push_constant_count;
        GPURootSignaturePoolID pool;

    } GPURootSignatureDescriptor;

    typedef struct GPUSamplerDescriptor
    {
        EGPUFilterType min_filter;
        EGPUFilterType mag_filter;
        EGPUMipMapMode mipmap_mode;
        EGPUAddressMode address_u;
        EGPUAddressMode address_v;
        EGPUAddressMode address_w;
        float mip_lod_bias;
        float max_anisotropy;
        EGPUCompareMode compare_func;
    } GPUSamplerDescriptor;

    typedef struct GPUSampler
    {
        GPUDeviceID device;
    } GPUSampler;

    typedef struct GPURootSignaturePool
    {
        GPUDeviceID device;
        EGPUPipelineType pipeline_type;
    } GPURootSignaturePool;

    typedef struct GPUParameterTable
    {
        // This should be stored here because shader could be destoryed after RS creation
        GPUShaderResource* resources;
        uint32_t resources_count;
        uint32_t set_index;
    } GPUParameterTable;

    typedef struct GPURootSignature
    {
        GPUDeviceID device;
        GPUParameterTable* tables;
        uint32_t table_count;
        CGPUShaderResource* push_constants;
        uint32_t push_constant_count;
        CGPUShaderResource* static_samplers;
        uint32_t static_sampler_count;
        EGPUPipelineType pipeline_type;
        GPURootSignaturePoolID pool;
        GPURootSignatureID pool_sig;
    } GPURootSignature;

    typedef struct GPUDepthStateDesc
    {
        bool depthTest;
        bool depthWrite;
        EGPUCompareMode depthFunc;
        bool stencilTest;
        uint8_t stencilReadMask;
        uint8_t stencilWriteMask;
        EGPUCompareMode stencilFrontFunc;
        EGPUStencilOp stencilFrontFail;
        EGPUStencilOp depthFrontFail;
        EGPUStencilOp stencilFrontPass;
        EGPUCompareMode stencilBackFunc;
        EGPUStencilOp stencilBackFail;
        EGPUStencilOp depthBackFail;
        EGPUStencilOp stencilBackPass;
    } GPUDepthStateDesc;

    typedef struct GPURasterizerStateDescriptor
    {
        EGPUCullMode cullMode;
        EGPUFillMode fillMode;
        EGPUFrontFace frontFace;
        int32_t depthBias;
        float slopeScaledDepthBias;
        bool enableMultiSample;
        bool enableScissor;
        bool enableDepthClamp;
    } GPURasterizerStateDescriptor;

    typedef struct GPUBlendStateDescriptor
    {
        /// Source blend factor per render target.
        EGPUBlendConstant srcFactors[GPU_MAX_MRT_COUNT];
        /// Destination blend factor per render target.
        EGPUBlendConstant dstFactors[GPU_MAX_MRT_COUNT];
        /// Source alpha blend factor per render target.
        EGPUBlendConstant srcAlphaFactors[GPU_MAX_MRT_COUNT];
        /// Destination alpha blend factor per render target.
        EGPUBlendConstant dstAlphaFactors[GPU_MAX_MRT_COUNT];
        /// Blend mode per render target.
        EGPUBlendMode blendModes[GPU_MAX_MRT_COUNT];
        /// Alpha blend mode per render target.
        EGPUBlendMode blendAlphaModes[GPU_MAX_MRT_COUNT];
        /// Write mask per render target.
        int32_t masks[GPU_MAX_MRT_COUNT];
        /// Set whether alpha to coverage should be enabled.
        bool alphaTCoverage;
        /// Set whether each render target has an unique blend function. When false the blend function in slot 0 will be used for all render targets.
        bool independentBlend;
    } GPUBlendStateDescriptor;

    typedef struct GPURenderPipelineDescriptor
    {
        GPURootSignatureID pRootSignature;
        const GPUShaderEntryDescriptor* pVertexShader;
        const GPUShaderEntryDescriptor* pFragmentShader;
        const GPUVertexLayout* pVertexLayout;
        const GPUDepthStateDesc* pDepthState;
        const GPURasterizerStateDescriptor* pRasterizerState;
        const GPUBlendStateDescriptor* pBlendState;

        EGPUSampleCount samplerCount;
        EGPUPrimitiveTopology primitiveTopology;

        EGPUFormat* pColorFormats;
        uint32_t renderTargetCount;
        EGPUFormat depthStencilFormat;
    } GPURenderPipelineDescriptor;

    typedef struct GPURenderPipeline
    {
        GPUDeviceID pDevice;
        GPURootSignatureID pRootSignature;
    } GPURenderPipeline;

    typedef struct GPUCommandPool
    {
        GPUQueueID queue;
    } GPUCommandPool;

    typedef struct GPUCommandBufferDescriptor
    {
        bool isSecondary : 1;
    } GPUCommandBufferDescriptor;


    typedef struct GPUCommandBuffer
    {
        GPUDeviceID device;
        GPUCommandPoolID pool;
        EGPUPipelineType currentDispatch;
    } GPUCommandBuffer;

    typedef struct GPUFence
    {
        GPUDeviceID device;
    } GPUFence;

    typedef struct GPUAcquireNextDescriptor
    {
        GPUSemaphoreID signal_semaphore;
        GPUFenceID fence;
    } GPUAcquireNextDescriptor;

    typedef struct GPUSemaphore
    {
        GPUDeviceID device;
    } GPUSemaphore;

    typedef struct GPUColorAttachment {
        GPUTextureViewID view;
        GPUTextureViewID resolve_view;
        EGPULoadAction load_action;
        EGPUStoreAction store_action;
        GPUClearValue clear_color;
    } GPUColorAttachment;

    typedef struct GPUDepthStencilAttachment {
        GPUTextureViewID view;
        EGPULoadAction depth_load_action;
        EGPUStoreAction depth_store_action;
        float clear_depth;
        uint8_t write_depth;
        EGPULoadAction stencil_load_action;
        EGPUStoreAction stencil_store_action;
        uint32_t clear_stencil;
        uint8_t write_stencil;
    } GPUDepthStencilAttachment;

    typedef struct GPURenderPassDescriptor
    {
        const char* name;
        // TODO: support multi-target & remove this
        EGPUSampleCount sample_count;
        const GPUColorAttachment* color_attachments;
        const GPUDepthStencilAttachment* depth_stencil;
        uint32_t render_target_count;
    } GPURenderPassDescriptor;

    typedef struct GPUTextureBarrier {
        GPUTextureID texture;
        EGPUResourceState src_state;
        EGPUResourceState dst_state;
        uint8_t queue_acquire;
        uint8_t queue_release;
        EGPUQueueType queue_type;
        /// Specifiy whether following barrier targets particular subresource
        uint8_t subresource_barrier;
        /// Following values are ignored if subresource_barrier is false
        uint8_t mip_level;
        uint16_t array_layer;
        uint8_t d3d12_begin_only;
        uint8_t d3d12_end_only;
    } GPUTextureBarrier;

    typedef struct GPUBufferDescriptor
    {
        uint64_t size;
        GPUResourceTypes descriptors; // bffer usage
        /// Memory usage
        /// Decides which memory heap buffer will use (default, upload, readback)
        EGPUMemoryUsage memory_usage;
        EGPUFormat format;
        EGPUResourceState start_state;/// What state will the buffer get created in
        GPUBufferCreationFlags flags;
        GPUQueueID owner_queue; /// Owner queue of the resource at creation
        bool prefer_on_device;
        bool prefer_on_host;
    } GPUBufferDescriptor;
    typedef struct GPUBuffer
    {
        GPUDeviceID device;
        /**
         * CPU address of the mapped buffer.
         * Applicable to buffers created in CPU accessible heaps (CPU, CPU_TO_GPU, GPU_TO_CPU)
         */
        void* cpu_mapped_address;
        uint64_t size : 37;
        uint64_t descriptors : 24;
        uint64_t memory_usage : 3;
    } GPUBuffer;

    typedef struct GPUBufferBarrier
    {
        GPUBufferID buffer;
        EGPUResourceState src_state;
        EGPUResourceState dst_state;
        uint8_t queue_acquire;
        uint8_t queue_release;
        EGPUQueueType queue_type;
        uint8_t d3d12_begin_only;
        uint8_t d3d12_end_only;
    } GPUBufferBarrier;

    typedef struct GPUResourceBarrierDescriptor
    {
        const GPUBufferBarrier* buffer_barriers;
        uint32_t buffer_barriers_count;
        const GPUTextureBarrier* texture_barriers;
        uint32_t texture_barriers_count;
    } GPUResourceBarrierDescriptor;

    typedef struct GPURenderPassEncoder {
        GPUDeviceID device;
    } GPURenderPassEncoder;

    typedef struct GPUQueueSubmitDescriptor
    {
        GPUCommandBufferID* cmds;
        GPUFenceID signal_fence;
        GPUSemaphoreID* wait_semaphores;
        GPUSemaphoreID* signal_semaphores;
        uint32_t cmds_count;
        uint32_t wait_semaphore_count;
        uint32_t signal_semaphore_count;
    } GPUQueueSubmitDescriptor;

    typedef struct GPUQueuePresentDescriptor
    {
        GPUSwapchainID swapchain;
        const GPUSemaphoreID* wait_semaphores;
        uint32_t wait_semaphore_count;
        uint8_t index;
    } GPUQueuePresentDescriptor;

    typedef struct GPUBufferRange
    {
        uint64_t offset;
        uint64_t size;
    } GPUBufferRange;

    typedef struct GPUBufferToBufferTransfer
    {
        GPUBufferID dst;
        uint64_t dst_offset;
        GPUBufferID src;
        uint64_t src_offset;
        uint64_t size;
    } GPUBufferToBufferTransfer;

    typedef struct GPUDescriptorSetDescriptor
    {
        GPURootSignatureID root_signature;
        uint32_t set_index;
    } GPUDescriptorSetDescriptor;

    typedef struct GPUDescriptorSet
    {
        GPURootSignatureID root_signature;
        uint32_t index;
    } GPUDescriptorSet;

    typedef struct GPUDescriptorData
    {
        // Update Via Shader Reflection.
        const char8_t* name;
        // Update Via Binding Slot.
        uint32_t binding;
        EGPUResourceType binding_type;
        union
        {
            struct
            {
                /// Offset to bind the buffer descriptor
                const uint64_t* offsets;
                const uint64_t* sizes;
            } buffers_params;
            // Descriptor set buffer extraction options
            // TODO: Support descriptor buffer extraction
            // struct
            //{
            //    struct CGPUShaderEntryDescriptor* shader;
            //    uint32_t buffer_index;
            //    ECGPUShaderStage shader_stage;
            //} extraction_params;
            struct
            {
                uint32_t uav_mip_slice;
                bool blend_mip_chain;
            } uav_params;
            bool enable_stencil_resource;
        };
        union
        {
            const void** ptrs;
            /// Array of texture descriptors (srv and uav textures)
            GPUTextureViewID* textures;
            /// Array of sampler descriptors
            GPUSamplerID* samplers;
            /// Array of buffer descriptors (srv, uav and cbv buffers)
            GPUBufferID* buffers;
            /// Array of pipeline descriptors
            GPURenderPipelineID* render_pipelines;
            /// Array of pipeline descriptors
            //GPUComputePipelineID* compute_pipelines;
            /// DescriptorSet buffer extraction
            GPUDescriptorSetID* descriptor_sets;
            /// Custom binding (raytracing acceleration structure ...)
            // CGPUAccelerationStructureId* acceleration_structures;
        };
        uint32_t count;
    } GPUDescriptorData;

#ifdef __cplusplus
}
#endif //extern "C"