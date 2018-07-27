#include "stdafx.h"
#include "CommandQueue.h"

namespace Fancy
{
  CommandListType CommandQueue::GetCommandListType(uint64 aFenceVal)
  {
    uint listTypeVal = aFenceVal >> 61;
    ASSERT(listTypeVal < (uint)CommandListType::NUM);

    return (CommandListType)listTypeVal;
  }

  CommandQueue::CommandQueue(CommandListType aType)
    : myType(aType)
    , myInitialFenceVal((((uint64) aType) << 61ULL))
  {
  }
  
  CommandQueue::~CommandQueue()
  {
  }
}

