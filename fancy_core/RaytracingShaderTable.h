#pragma once
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "RaytracingPipelineState.h"

namespace Fancy
{
  struct RaytracingShaderTableProperties
  {
    uint myNumRaygenShaderRecords = 0;
    uint myNumMissShaderRecords = 0;
    uint myNumHitShaderRecords = 0;
    uint myLocalCbufferOverallSize = 1 * SIZE_MB;
  };

  struct RaytracingShaderTableRange
  {
    GpuBuffer* myLocalCbuffer = nullptr;
    GpuBuffer* myBuffer = nullptr;
    uint64 myOffset = 0ull;
    uint64 mySize = 0ull;
    uint myStride = 0u;
  };

  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(const RaytracingShaderTableProperties& someProps);
    uint AddShaderRecord(const RaytracingShaderIdentifier& aShaderIdentifier, void* aCbufferData = nullptr, uint64 aCbufferDataSize = 0u);
    void SetLocalCbufferData_WaitForGPU(RaytracingShaderIdentifierType aType, uint aShaderRecordIndex, void* aCbufferData, uint64 aCbufferDataSize);
    
    RaytracingShaderTableRange GetRayGenRange() const { ASSERT(myProperties.myNumRaygenShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_RAYGEN); }
    RaytracingShaderTableRange GetMissRange() const { ASSERT(myProperties.myNumMissShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_MISS); }
    RaytracingShaderTableRange GetHitRange() const { ASSERT(myProperties.myNumHitShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_HIT); }
    
  protected:
    RaytracingShaderTableRange GetRange(RaytracingShaderIdentifierType aType) const;

    RaytracingShaderTableProperties myProperties;
    SharedPtr<GpuBuffer> myBuffer;
    GpuRingBuffer myCbuffer;
    
    uint8* myMappedData;
    uint myShaderIdentifierSizeBytes;
    uint myAlignedShaderRecordSizeBytes;
    
    uint myTypeRangeOffsets[RT_SHADER_IDENTIFIER_TYPE_NUM];
    uint myTypeRangeCurrSizes[RT_SHADER_IDENTIFIER_TYPE_NUM];
    uint myTypeRangeMaxSizes[RT_SHADER_IDENTIFIER_TYPE_NUM];
  };
}
