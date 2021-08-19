#pragma once
#include "GpuBuffer.h"
#include "RaytracingPipelineState.h"

namespace Fancy
{
  struct RaytracingShaderTableProperties
  {
    uint myNumRaygenShaderRecords = 0;
    uint myNumMissShaderRecords = 0;
    uint myNumHitShaderRecords = 0;
  };

  struct RaytracingShaderTableRange
  {
    GpuBuffer* myBuffer = nullptr;
    uint64 myOffset = 0ull;
    uint64 mySize = 0ull;
    uint myStride = 0u;
  };

  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(const RaytracingShaderTableProperties& someProps);
    uint AddShaderRecord(const RaytracingShaderRecord& aShaderRecord);
    
    RaytracingShaderTableRange GetRayGenRange() const { ASSERT(myProperties.myNumRaygenShaderRecords > 0); return GetRange(RT_SHADER_RECORD_TYPE_RAYGEN); }
    RaytracingShaderTableRange GetMissRange() const { ASSERT(myProperties.myNumMissShaderRecords > 0); return GetRange(RT_SHADER_RECORD_TYPE_MISS); }
    RaytracingShaderTableRange GetHitRange() const { ASSERT(myProperties.myNumHitShaderRecords > 0); return GetRange(RT_SHADER_RECORD_TYPE_HIT); }

  protected:
    RaytracingShaderTableRange GetRange(RaytracingShaderRecordType aType) const {
      return { myBuffer.get(), myTypeRangeOffsets[aType], myTypeRangeSizes[aType], myShaderRecordSizeBytes };
    }

    RaytracingShaderTableProperties myProperties;
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myMappedData;
    uint myShaderRecordSizeBytes;
    uint myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_NUM];
    uint myTypeRangeSizes[RT_SHADER_RECORD_TYPE_NUM];
    uint myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_NUM];
  };
}
