#pragma once

#include "GpuFence.h"
#include "DX12Prerequisites.h"

class CommandQueueDX12;

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuFenceDX12 final : public GpuFence
  {
  public:
    explicit GpuFenceDX12(CommandListType aCommandListType);
    ~GpuFenceDX12() override = default;

    bool IsDone(uint64 aFenceVal) override;
    uint64 SignalAndIncrement() override;
    void Wait(uint64 aFenceVal) override;
	
  protected:
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;

	ID3D12CommandQueue* GetCommandQueue() const;
  };
//---------------------------------------------------------------------------//
}

