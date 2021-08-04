#pragma once
#include "RaytracingShaderTable.h"

namespace Fancy
{
  class RaytracingShaderTableDX12 final : public RaytracingShaderTable
  {
  public:
    RaytracingShaderTableDX12(RaytracingShaderTableType aType, uint aMaxNumShaderRecords, const SharedPtr<RaytracingPipelineState>& anRtPso);
    uint AddShaderRecord(uint aShaderIndexInRtPso) override;

  private:
    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> myRtPsoProperties;
  };
}



