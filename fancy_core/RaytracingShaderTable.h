#pragma once

namespace Fancy
{
  class RaytracingShaderTable
  {
  public:
    RaytracingShaderTable(RaytracingShaderTableType aType, uint aMaxNumShaderRecords, const SharedPtr<RaytracingPipelineState>& anRtPso);
    virtual uint AddShaderRecord(uint aShaderIndexInRtPso) = 0;

  protected:
    RaytracingShaderTableType myType;
    uint myMaxNumShaderRecords;
    uint myShaderRecordSizeBytes;
    
    const SharedPtr<RaytracingPipelineState> myRtPso;
    SharedPtr<GpuBuffer> myBuffer;
    void* myMappedData;
    uint myWriteOffset;
  };
}
