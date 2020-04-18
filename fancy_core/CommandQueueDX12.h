#pragma once
#include "CommandQueue.h"
#include "WindowsIncludes.h"

#include <wrl.h>

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class CommandList;

  class CommandQueueDX12 final : public CommandQueue
  {
  public:
    CommandQueueDX12(CommandListType aType);
    ~CommandQueueDX12() = default;

    bool IsFenceDone(uint64 aFenceVal) override;
    // Waits for a fence-completion on CPU timeline
    void WaitForFence(uint64 aFenceVal) override;
    void WaitForIdle() override;
    // Waits for a fence-completion on GPU timeline
    void StallForQueue(const CommandQueue* aCommandQueue) override;
    void StallForFence(uint64 aFenceVal) override;
    uint64 ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode = SyncMode::ASYNC) override;
    uint64 ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode = SyncMode::ASYNC) override;
    void ResolveResourceHazardData(CommandList* aCommandList);
	
	  Microsoft::WRL::ComPtr<ID3D12CommandQueue> myQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
    HANDLE myFenceCompletedEvent;

  protected:
    uint64 SignalAndIncrementFence();
  };
}

#endif