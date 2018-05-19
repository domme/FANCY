#pragma once

#include "CommandListType.h"
#include "Fence.h"

namespace Fancy
{
  class CommandQueue
  {
  public:
    explicit CommandQueue(CommandListType aType);
    virtual ~CommandQueue();

  protected:
    CommandListType myType;
    UniquePtr<Fence> myFence;
  };
}

