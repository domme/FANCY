#pragma once
#include "GpuQueryHeap.h"
#include "DX12Prerequisites.h"

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


