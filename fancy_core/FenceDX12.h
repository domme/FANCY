#pragma once

#include "Fence.h"
#include "DX12Prerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class FenceDX12 final : public Fence
  {
  public:
    explicit FenceDX12(CommandListType aCommandListType);
    ~FenceDX12() override;

    bool IsDone(uint64 aFenceVal) override;
    void Signal(uint64 aFenceVal = 0u) override;
    void Wait(uint64 aFenceVal = 0u) override;

  protected:
    ID3D12CommandQueue* myCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
  };
//---------------------------------------------------------------------------//
}

