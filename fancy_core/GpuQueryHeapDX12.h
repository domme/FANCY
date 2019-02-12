#pragma once
#include "GpuQueryHeap.h"
#include "DX12Prerequisites.h"

namespace Fancy
{
  class GpuQueryHeapDX12 final : public GpuQueryHeap
  {
  public:
    GpuQueryHeapDX12(QueryType aQueryType, uint aNumQueries);
    ~GpuQueryHeapDX12() override = default;

  protected:
    Microsoft::WRL::ComPtr<ID3D12QueryHeap> myHeap;
  };
}


