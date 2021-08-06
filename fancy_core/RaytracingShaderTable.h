#pragma once
#include "RaytracingPipelineState.h"

namespace Fancy
{
  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(RaytracingShaderTableType aType, uint aMaxNumShaderRecords, const SharedPtr<RaytracingPipelineState>& anRtPso);
    uint AddShaderRecord(uint aShaderIndexInRtPso);

    GpuBuffer* GetBuffer(uint& aWrittenSizeOut) const { aWrittenSizeOut = myWriteOffset; return myBuffer.get(); }

  protected:
    virtual void AddShaderRecordInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry) = 0;
    virtual void AddShaderRecordInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aHitGroup) = 0;

    RaytracingShaderTableType myType;
    uint myMaxNumShaderRecords;
    uint myShaderRecordSizeBytes;
    
    const SharedPtr<RaytracingPipelineState> myRtPso;
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myMappedData;
    uint myWriteOffset;

#if FANCY_HEAVY_DEBUG
    eastl::vector<uint> myShaders;
#endif
  };
}
