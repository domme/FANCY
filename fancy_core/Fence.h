#pragma once

#include "CommandListType.h"
#include "DX12Prerequisites.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  class Fence
  {
  public:
    explicit Fence(CommandListType aCommandListType);
    virtual ~Fence();

    virtual bool IsDone(uint64 aFenceVal) = 0;
    virtual void Signal(uint64 aFenceVal = 0u) = 0;
    virtual void Wait(uint64 aFenceVal = 0u) = 0;

  protected:
    uint64 myLastCompletedVal; 
    uint64 myNextVal;
    CommandListType myCommandListType;
    HANDLE myEventHandle;
  };
  //---------------------------------------------------------------------------//
} 