#pragma once
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "RtPipelineState.h"

namespace Fancy
{
  struct RtShaderBindingTableProperties
  {
    uint myNumRaygenShaderRecords = 0;
    uint myNumMissShaderRecords = 0;
    uint myNumHitShaderRecords = 0;
  };

  struct RtShaderBindingTableRange
  {
    GpuBuffer* mySbtBuffer = nullptr;
    uint64 myOffset = 0ull;
    uint64 mySize = 0ull;
    uint myStride = 0u;
  };

  class RtShaderBindingTable
  {
  public:
    RtShaderBindingTable(const RtShaderBindingTableProperties& someProps);
    uint AddShaderRecord(const RtShaderIdentifier& aShaderIdentifier);
    
    RtShaderBindingTableRange GetRayGenRange() const { ASSERT(myProperties.myNumRaygenShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_RAYGEN); }
    RtShaderBindingTableRange GetMissRange() const { ASSERT(myProperties.myNumMissShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_MISS); }
    RtShaderBindingTableRange GetHitRange() const { ASSERT(myProperties.myNumHitShaderRecords > 0); return GetRange(RT_SHADER_IDENTIFIER_TYPE_HIT); }
    
  protected:
    RtShaderBindingTableRange GetRange(RtShaderIdentifierType aType) const;

    RtShaderBindingTableProperties myProperties;
    SharedPtr<GpuBuffer> mySbtBuffer;

    uint8* myMappedSbtData;
    uint myAlignedShaderRecordSizeBytes;

    uint myRecordTypeOffset[RT_SHADER_IDENTIFIER_TYPE_NUM];
    uint myNumUsedRecords[RT_SHADER_IDENTIFIER_TYPE_NUM];
    uint myMaxNumRecords[RT_SHADER_IDENTIFIER_TYPE_NUM];
  };
}
