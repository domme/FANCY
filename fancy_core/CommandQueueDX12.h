#pragma once
#include "CommandQueue.h"

namespace Fancy
{
  class CommandQueueDX12 : public CommandQueue
  {
  public:
    CommandQueueDX12(CommandListType aType);
    ~CommandQueueDX12() = default;
	
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> myQueue;
  };
}

