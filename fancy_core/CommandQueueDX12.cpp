#include "stdafx.h"
#include "CommandQueueDX12.h"
#include "FenceDX12.h"

namespace Fancy
{
  CommandQueueDX12::CommandQueueDX12(CommandListType aType)
    : CommandQueue(aType)
  {
    myFence.reset(new FenceDX12);
  }
  
  CommandQueueDX12::~CommandQueueDX12()
  {
  }

  
}


