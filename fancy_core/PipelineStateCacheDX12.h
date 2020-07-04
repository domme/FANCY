#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "RenderPlatformObjectCache.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  struct GraphicsPipelineState;
  struct ComputePipelineState;

  class PipelineStateCacheDX12 : public RenderPlatformObjectCache<ID3D12PipelineState*>
  {
  public:
    ~PipelineStateCacheDX12() override;

    ID3D12PipelineState* GetCreateGraphicsPSO(const GraphicsPipelineState& aState);
    ID3D12PipelineState* GetCreateComputePSO(const ComputePipelineState& aState);
    void Clear() override;
  };
}

#endif