#include "stdafx.h"
#include "CommandQueue.h"

namespace Fancy
{
  CommandQueue::CommandQueue(CommandListType aType)
    : myType(aType)
  {
  }
  
  CommandQueue::~CommandQueue()
  {
  }
}

