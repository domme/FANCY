#pragma once
#include "RaytracingPipelineState.h"

namespace Fancy
{
  struct RaytracingShaderTableProperties
  {
    uint myNumRaygenShaderRecords = 0;
    uint myNumMissShaderRecords = 0;
    uint myNumHitShaderRecords = 0;
  };

  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(const RaytracingShaderTableProperties& someProps, const SharedPtr<RaytracingPipelineState>& anRtPso);
    uint AddShaderRecord(RaytracingShaderRecordType aType, uint aShaderIndexInRtPso);
    
    GpuBuffer* GetBufferRange(RaytracingShaderRecordType aType, uint64& anOffsetOut, uint64& aSizeOut) const
    {
      anOffsetOut = myTypeRangeOffsets[aType];
      aSizeOut = myTypeRangeSizes[aType];
      return myBuffer.get();
    }

  protected:
    virtual void GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry, eastl::fixed_vector<uint8, 64>& someDataOut) = 0;
    virtual void GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aShaderEntry, eastl::fixed_vector<uint8, 64>& someDataOut) = 0;

    RaytracingShaderTableProperties myProperties;
    const SharedPtr<RaytracingPipelineState> myRtPso;
    SharedPtr<GpuBuffer> myBuffer;
    uint8* myMappedData;
    uint myShaderRecordSizeBytes;
    uint myTypeRangeOffsets[RT_SHADER_RECORD_TYPE_NUM];
    uint myTypeRangeSizes[RT_SHADER_RECORD_TYPE_NUM];
    uint myTypeRangeMaxSizes[RT_SHADER_RECORD_TYPE_NUM];

#if FANCY_HEAVY_DEBUG
    eastl::vector<eastl::pair<RaytracingShaderRecordType, uint>> myShaders;
#endif
  };
}
