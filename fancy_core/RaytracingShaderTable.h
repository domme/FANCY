#pragma once
#include "GpuBuffer.h"
#include "GpuRingBuffer.h"
#include "RaytracingPipelineState.h"

namespace Fancy
{
  struct RaytracingShaderTableProperties
  {
    RaytracingShaderIdentifierType myType = RT_SHADER_IDENTIFIER_TYPE_NUM;
    uint myMaxNumRecords = 0;
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
    
    RaytracingShaderTableRange GetRange() const;
    
  protected:
    RaytracingShaderTableProperties myProperties;
    SharedPtr<GpuBuffer> mySbtBuffer;

    uint8* myMappedSbtData;
    uint myAlignedShaderRecordSizeBytes;
    uint64 mySbtOffset;
  };
}
