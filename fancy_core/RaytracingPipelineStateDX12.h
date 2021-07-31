#pragma once
#include "RaytracingPipelineState.h"
#include "DX12Prerequisites.h"
#include "FancyCoreDefines.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class RaytracingPipelineStateDX12 final : public RaytracingPipelineState
  {
  public:
    RaytracingPipelineStateDX12(const RaytracingPipelineStateProperties& someProps);

    bool Recompile() override;

    Microsoft::WRL::ComPtr<ID3D12StateObject> myStateObject;
  };
}

#endif