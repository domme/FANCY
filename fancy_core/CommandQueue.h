#pragma once

#include "CommandListType.h"

namespace Fancy 
{
  class CommandContext;

  class CommandQueue
  {
  public:
    explicit CommandQueue(CommandListType aType);
    virtual ~CommandQueue();

    virtual bool IsFenceDone(uint64 aFenceVal) = 0;
    virtual uint64 SignalAndIncrementFence() = 0;
    virtual void WaitForFence(uint64 aFenceVal) = 0;
    virtual void WaitForIdle() = 0;
    virtual uint64 ExecuteCommandContext(CommandContext* aContext, bool aWaitForCompletion = false) = 0;

  protected:
    CommandListType myType;
  };
}

