#pragma once
#include "Rendering/RtPipelineState.h"
#include "DX12Prerequisites.h"
#include "Common/FancyCoreDefines.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class RtPipelineStateDX12 final : public RtPipelineState
  {
  public:
    RtPipelineStateDX12(const RtPipelineStateProperties& someProps);

    bool Recompile() override;

    Microsoft::WRL::ComPtr<ID3D12StateObject> myStateObject;
    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> myRtPsoProperties;

  protected:
    void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::HitGroup& aShaderEntry, RtShaderIdentifier& someDataOut) override;
    void GetShaderIdentifierDataInternal(uint aShaderIndexInRtPso, const RtPipelineStateProperties::ShaderEntry& aShaderEntry, RtShaderIdentifier& someDataOut) override;
  };
}

#endif