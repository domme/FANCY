#pragma once
#include "RaytracingShaderTable.h"

namespace Fancy
{
  class RaytracingShaderTableDX12 final : public RaytracingShaderTable
  {
  public:
    RaytracingShaderTableDX12(const RaytracingShaderTableProperties& someProps, const SharedPtr<RaytracingPipelineState>& anRtPso);

  private:
    void GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::ShaderEntry& aShaderEntry, eastl::fixed_vector<uint8, 64>& someDataOut) override;
    void GetShaderRecordDataInternal(uint aShaderIndexInRtPso, const RaytracingPipelineStateProperties::HitGroup& aShaderEntry, eastl::fixed_vector<uint8, 64>& someDataOut) override;

    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> myRtPsoProperties;
  };
}
