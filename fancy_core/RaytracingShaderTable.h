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
    uint myMaxCbufferSize = 1 * SIZE_MB;
  };

  struct RaytracingShaderTableRange
  {
    GpuBuffer* myLocalCbuffer = nullptr;
    GpuBuffer* mySbtBuffer = nullptr;
    uint64 myOffset = 0ull;
    uint64 mySize = 0ull;
    uint myStride = 0u;
  };

  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(const RaytracingShaderTableProperties& someProps);
    uint AddShaderRecord(const RaytracingShaderIdentifier& aShaderIdentifier, uint64 aCbufferDataSize = 0u, void* aCbufferData = nullptr);
    void UpdateCbufferData(CommandList* aCommandList, uint aRecordIdx, uint64 aSize, void* aData);
    
    RaytracingShaderTableRange GetRange() const;
    
  protected:
    RaytracingShaderTableProperties myProperties;
    SharedPtr<GpuBuffer> mySbtBuffer;
    SharedPtr<GpuBuffer> myCbuffer;
    eastl::vector<eastl::pair<uint64, uint64>> myCbufferRangePerRecord;

    uint8* myMappedSbtData;
    
    uint myShaderIdentifierSizeBytes;
    uint myAlignedShaderRecordSizeBytes;

    uint64 mySbtOffset;
    uint64 myCbufferOffset;
  };
}
