#pragma once
#include "Rendering/GpuQueryHeap.h"
#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class GpuQueryHeapDX12 final : public GpuQueryHeap
  {
  public:
    GpuQueryHeapDX12(GpuQueryType aQueryType, uint aNumQueries);
    ~GpuQueryHeapDX12() override;

    Microsoft::WRL::ComPtr<ID3D12QueryHeap> myHeap;
  };
}

#endif