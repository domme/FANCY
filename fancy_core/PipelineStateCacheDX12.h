#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "RenderPlatformObjectCache.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  struct GraphicsPipelineState;
  struct ComputePipelineState;

  class PipelineStateCacheDX12
  {
  public:
    ~PipelineStateCacheDX12();

    ID3D12PipelineState* GetCreateGraphicsPSO(const GraphicsPipelineState& aState);
    ID3D12PipelineState* GetCreateComputePSO(const ComputePipelineState& aState);
    
    void Clear();

  private:
    std::mutex myCacheMutex;
    eastl::hash_map<uint64, ID3D12PipelineState*> myGraphicsPsoCache;
    eastl::hash_map<uint64, ID3D12PipelineState*> myComputePsoCache;
  };
}

#endif