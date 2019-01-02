#pragma once

#include "FancyCoreDefines.h"
#include "RenderEnums.h"

namespace Fancy 
{
  class CommandContext;

  class CommandQueue
  {
  public:
    static CommandListType GetCommandListType(uint64 aFenceVal);

    explicit CommandQueue(CommandListType aType);
    virtual ~CommandQueue() = default;

    virtual bool IsFenceDone(uint64 aFenceVal) = 0;
    virtual uint64 SignalAndIncrementFence() = 0;
    virtual void WaitForFence(uint64 aFenceVal) = 0;
    virtual void WaitForIdle() = 0;
    virtual uint64 ExecuteContext(CommandContext* aContext, bool aWaitForCompletion = false) = 0;

  protected:
    CommandListType myType;
    uint64 myInitialFenceVal;
  };
}