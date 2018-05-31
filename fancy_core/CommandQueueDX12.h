#pragma once
#include "CommandQueue.h"

namespace Fancy
{
  class CommandContext;

  class CommandQueueDX12 : public CommandQueue
  {
  public:
    CommandQueueDX12(CommandListType aType);
    ~CommandQueueDX12() = default;

    bool IsFenceDone(uint64 aFenceVal);
    uint64 SignalAndIncrementFence();
    void WaitForFence(uint64 aFenceVal);
    void WaitForIdle();

    uint64 ExecuteCommandContext(CommandContext* aContext, bool aWaitForCompletion);
	
	  Microsoft::WRL::ComPtr<ID3D12CommandQueue> myQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
    uint64 myLastCompletedFenceVal;
    uint64 myNextFenceVal;
    HANDLE myFenceCompletedEvent;
  };
}

