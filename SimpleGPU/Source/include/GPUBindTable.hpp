#pragma once
#include "api.h"
#include <string>
#include <vector>

DEFINE_GPU_OBJECT(GPUBindTable)

typedef struct GPUXBindTableDescriptor
{
    GPURootSignatureID pRootSignature;
    const char** ppNames;
    uint32_t namesCount;
} GPUXBindTableDescriptor;

struct GPUBindTableLocation;
typedef struct GPUBindTableValue
{
    friend struct GPUBindTable;
public:
    GPUBindTableValue() = default;
    GPUBindTableValue(const GPUBindTableValue&) = delete;
    GPUBindTableValue& operator=(const GPUBindTableValue&) = delete;

    void Initialize(const GPUBindTableLocation& loc, const GPUDescriptorData& rhs);

protected:
    bool mBinded            = false;
    GPUDescriptorData mData = {};
    // arena
    std::vector<const void*> mResources;
} GPUBindTableValue;

typedef struct GPUBindTableLocation
{
    const uint32_t tableIndex = 0; // set index
    const uint32_t binding    = 0;
    GPUBindTableValue mValue;
} GPUBindTableLocation;

typedef struct GPUBindTable
{
    static GPUBindTableID Create(GPUDeviceID device, const GPUXBindTableDescriptor* desc);
    static void Free(GPUBindTableID table);

    void Bind(GPURenderPassEncoderID encoder) const;
    void Update(const GPUDescriptorData* pData, uint32_t count);

    GPURootSignatureID m_pRS               = nullptr;
    uint64_t* m_pNamesHash                 = nullptr;
    GPUBindTableLocation* m_pNamesLocation = nullptr;
    uint32_t mNamesCount                   = 0;
    uint32_t mSetsCount                    = 0;
    GPUDescriptorSetID* m_ppSets           = nullptr;

private:
    void UpdateSelfIfDirty();

} GPUBindTable;


template <typename T> struct EqualTo;

template <>
struct EqualTo<GPUDescriptorData>
{
    bool operator()(const GPUDescriptorData& a, const GPUDescriptorData& b);
};

/////////////////////////
GPUBindTableID GPUCreateBindTable(GPUDeviceID device, const GPUXBindTableDescriptor* desc);
void GPUFreeBindTable(GPUBindTableID table);
void GPUBindTableUpdate(GPUBindTableID table, const GPUDescriptorData* datas, uint32_t count);
void GPURenderEncoderBindBindTable(GPURenderPassEncoderID encoder, GPUBindTableID table);
