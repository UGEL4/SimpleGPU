#pragma once
#include "gpuconfig.h"
#include <stdint.h>

#if defined(_MSC_VER) && !defined(__clang__)
    #define DECLEAR_ZERO_VAL(type, var, num)              \
        type* var = (type*)_alloca(sizeof(type) * (num)); \
        memset((void*)var, 0, sizeof(type) * (num));
#else
    #define DECLEAR_ZERO_VAL(type, var, num) \
        type* var[(num)];                    \
        memset((void*)var, 0, sizeof(type) * (num));
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
		GPU_FORMT_COUNT
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
        GPU_SHADER_STAGE_COUNT        = 6,
        GPU_SHADER_STAGE_MAX_ENUM_BIT = 0x7FFFFFFF
    } EGPUShaderStage;

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
    GPUQueueID GPUGetQueue(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);
    typedef GPUQueueID (*GPUProcGetQueue)(GPUDeviceID pDevice, EGPUQueueType queueType, uint32_t queueIndex);


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

    //shader api
    GPUShaderLibraryID GPUCreateShaderLibrary(GPUDeviceID pDevice, GPUShaderLibraryDescriptor* pDesc);
    typedef GPUShaderLibraryID (*GPUProcCreateShaderLibrary)(GPUDeviceID pDevice, GPUShaderLibraryDescriptor* pDesc);
    void GPUFreeShaderLibrary(GPUShaderLibraryID pShader);
    typedef void (*GPUProcFreeShaderLibrary)(GPUShaderLibraryID pShader);

    // pipeline
    GPURenderPipelineID GPUCreateRenderPipeline(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc);
    typedef GPURenderPipelineID (*GPUProcCreateRenderPipeline)(GPUDeviceID pDevice, const GPURenderPipelineDescriptor* pDesc);
    void GPUFreeRenderPipeline(GPURenderPipelineID pPipeline);
    typedef void (*GPUProcFreeRenderPipeline)(GPURenderPipelineID pPipeline);

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
        const GPUProcGetQueue GetQueue;

        // swapchain api
        const GPUProcCreateSwapchain CreateSwapchain;
        const GPUProcFreeSwapchain FreeSwapchain;

        // texture & texture_view api
        const GPUProcCreateTextureView CreateTextureView;
        const GPUProcFreeTextureView FreeTextureView;

        //shader
        const GPUProcCreateShaderLibrary CreateShaderLibrary;
        const GPUProcFreeShaderLibrary FreeShaderLibrary;

        //pipeline
        const GPUProcCreateRenderPipeline CreateRenderPipeline;
        const GPUProcFreeRenderPipeline FreeRenderPipeline;
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

	typedef struct GPUAdapterDetail
	{

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
        uint32_t canClias : 1;
        uint32_t isImported : 1;
        uint32_t canExport : 1;
        void* nativeHandle;
        uint64_t uniqueId;
    } GPUTexture;

    typedef struct GPUTextureViewDescriptor
    {
        GPUTextureID pTexture;
        EGPUFormat format;
        uint32_t usage;
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

    typedef struct GPUShaderLibraryDescriptor
    {
        const char8_t* pName;
        const uint32_t* code;
        uint32_t codeSize;
        EGPUShaderStage stage;
        //bool reflection_only;
    } GPUShaderLibraryDescriptor;

    typedef struct GPUShaderReflection
    {
        /*const char8_t* entry_name;
        ECGPUShaderStage stage;
        CGPUVertexInput* vertex_inputs;
        CGPUShaderResource* shader_resources;
        uint32_t vertex_inputs_count;
        uint32_t shader_resources_count;
        uint32_t thread_group_sizes[3];*/
    } GPUShaderReflection;

    typedef struct GPUShaderLibrary
    {
        GPUDeviceID pDevice;
       /* char8_t* name;
        GPUShaderReflection* entry_reflections;
        uint32_t entrys_count;*/
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

    typedef struct GPURootSignature
    {

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
        int32_t depthBias;
        float slopeScaledDepthBias;
        EGPUFillMode fillMode;
        EGPUFrontFace frontFace;
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

#ifdef __cplusplus
}
#endif //extern "C"