#pragma once
#include "CommandQueue.h"
#include "WindowsIncludes.h"

#include <wrl.h>

namespace Fancy
{
  class CommandContext;

  class CommandQueueDX12 final : public CommandQueue
  {
  public:
    CommandQueueDX12(CommandListType aType);
    ~CommandQueueDX12() = default;

    bool IsFenceDone(uint64 aFenceVal) override;
    uint64 SignalAndIncrementFence() override;
    // Waits for a fence-completion on CPU timeline
    void WaitForFence(uint64 aFenceVal) override;
    void WaitForIdle() override;
    // Waits for a fence-completion on GPU timeline
    void StallForQueue(const CommandQueue* aCommandQueue) override;
    void StallForFence(uint64 aFenceVal) override;
    uint64 ExecuteContext(CommandContext* aContext, bool aWaitForCompletion = false) override;
	
	  Microsoft::WRL::ComPtr<ID3D12CommandQueue> myQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
    HANDLE myFenceCompletedEvent;
  };
}