#pragma once

#include "CommandListType.h"
#include "GpuFence.h"

namespace Fancy
{
  class CommandQueue
  {
  public:
    explicit CommandQueue(CommandListType aType);
    virtual ~CommandQueue();

  protected:
    CommandListType myType;
    UniquePtr<GpuFence> myFence;
  };
}

