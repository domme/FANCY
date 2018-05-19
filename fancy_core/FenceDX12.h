#pragma once

#include "Fence.h"
#include "DX12Prerequisites.h"


namespace Fancy {
//---------------------------------------------------------------------------//
  class FenceDX12 final : public Fence
  {
  public:
    FenceDX12(CommandListType aCommandListType);
    ~FenceDX12() override;

    void Wait() override;
    bool IsDone(uint64 anOtherFenceVal) override;
    
  protected:
    void Wait_Internal() override;

    Microsoft::WRL::ComPtr<ID3D12Fence> myGpuFence;

  };
//---------------------------------------------------------------------------//
}

