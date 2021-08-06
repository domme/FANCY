#pragma once
#include "RaytracingShaderTable.h"

namespace Fancy
{
  class RaytracingShaderTableDX12 final : public RaytracingShaderTable
  {
  public:
    RaytracingShaderTableDX12(RaytracingShaderTableType aType, uint aMaxNumShaderRecords, const SharedPtr<RaytracingPipelineState>& anRtPso);

  private:
    void AddShaderRecordInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aHitGroup) override;
    void AddShaderRecordInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry) override;

    void WriteShaderIdentifier(const wchar_t* aUniqueName);

    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> myRtPsoProperties;
  };
}
