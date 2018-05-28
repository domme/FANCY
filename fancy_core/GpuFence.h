#pragma once

#include "CommandListType.h"
#include "DX12Prerequisites.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class GpuFence
  {
  public:
    explicit GpuFence(CommandListType aCommandListType);
    virtual ~GpuFence();

    virtual bool IsDone(uint64 aFenceVal) = 0;
    virtual uint64 SignalAndIncrement() = 0;
    virtual void Wait(uint64 aFenceVal = 0u) = 0;

  protected:
    uint64 myLastCompletedVal; 
    uint64 myNextVal;
    CommandListType myCommandListType;
    HANDLE myEventHandle;
  };
  //---------------------------------------------------------------------------//
} 