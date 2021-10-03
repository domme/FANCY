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
  };

  struct RaytracingShaderTableRange
  {
    GpuBuffer* mySbtBuffer = nullptr;
    uint64 myOffset = 0ull;
    uint64 mySize = 0ull;
    uint myStride = 0u;
  };

  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(const RaytracingShaderTableProperties& someProps);
    uint AddShaderRecord(const RaytracingShaderIdentifier& aShaderIdentifier);
    
    RaytracingShaderTableRange GetRayGenRange() const { ASSERT(myProperties.myNumRaygenShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_RAYGEN); }
    RaytracingShaderTableRange GetMissRange() const { ASSERT(myProperties.myNumMissShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_MISS); }
    RaytracingShaderTableRange GetHitRange() const { ASSERT(myProperties.myNumHitShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_HIT); }
    
  protected:
    RaytracingShaderTableRange GetRange(RaytracingShaderIdentifierType aType) const;

    RaytracingShaderTableProperties myProperties;
    SharedPtr<GpuBuffer> mySbtBuffer;

    uint8* myMappedSbtData;
    uint myAlignedShaderRecordSizeBytes;

    uint myRecordTypeOffset[RT_SHADER_IDENTIFIER_TYPE_NUM];
    uint myNumUsedRecords[RT_SHADER_IDENTIFIER_TYPE_NUM];
    uint myMaxNumRecords[RT_SHADER_IDENTIFIER_TYPE_NUM];
  };
}
