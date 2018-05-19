#pragma once

#include "CommandListType.h"
#include "DX12Prerequisites.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class Fence
  {
  public:
    Fence(CommandListType aCommandListType);
    virtual ~Fence();
    
    virtual void SignalOnCpu(uint64 aFenceVal = UINT64_MAX);
    virtual void SignalOnGpu(uint64 aFenceVal = UINT64_MAX);
    virtual void WaitWithCpu(uint64 aFenceVal = UINT64_MAX);
    virtual void WaitWithGpu(uint64 aFenceVal = UINT64_MAX);
    virtual bool IsDone(uint64 aFenceVal = UINT64_MAX);

  protected:
    HANDLE myIsDoneEvent;
    uint64 myVal;  // The last completed value
    CommandListType myCommandListType;

    Microsoft::WRL::ComPtr<ID3D12Fence> myGpuFence;
  };
  //---------------------------------------------------------------------------//
}