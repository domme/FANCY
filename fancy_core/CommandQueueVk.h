#pragma once
#include "CommandQueue.h"
#include "CircularArray.h"

#include "VkPrerequisites.h"

namespace Fancy
{
  class CommandQueueVk final : public CommandQueue
  {
  public:
    CommandQueueVk(CommandListType aType);
    virtual ~CommandQueueVk();

    bool IsFenceDone(uint64 aFenceVal) override;
    void WaitForFence(uint64 aFenceVal) override;
    void WaitForIdle() override;
    void StallForQueue(const CommandQueue* aCommandQueue) override;
    void StallForFence(uint64 aFenceVal) override;

    VkQueue GetQueue() const { return myQueue; }

  protected:
    // The SyncPoints and the Semaphore-wrapper emulate a semaphore with a running integer payload
    // similar to what DX12 has and what the high-level rendering API expects when passing around increasing UINT64 fence-values.
    // As soon as the extension VK_KHR_timeline_semaphore becomes available on all major Vendors (Intel is still missing at this time...)
    // we can switch to that to arrive at a simpler API.

    // Each VkSemaphore can only be waited on once because a wait-operation resets a semaphore again to the unsignalled state.
    // That's why each SyncPoint includes multiple semaphores for each other queue-type.
    // With that design, each other queue can wait on a commandList-submission once.
    struct Semaphore
    {
      VkSemaphore mySemaphores[(uint)CommandListType::NUM] = { nullptr };
      bool myWasWaitedOn[(uint)CommandListType::NUM] = { false };
    };

    struct SyncPoint
    {
      Semaphore mySemaphore;  // GPU-GPU syncs
      VkFence myFence;        // CPU-GPU syncs
      uint64 myWaitingOnVal;
    };

    uint64 ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode) override;
    uint64 ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode) override;

    SyncPoint* GetNewSyncPoint();
    void AddPendingWaitSemaphore(Semaphore* aWaitSemaphore);

    void RecreateSemaphores(Semaphore& aSemaphore);

    mutable StaticCircularArray<SyncPoint, 8> mySyncPoints;
    // mutable StaticCircularArray<SyncPoint, 256> mySyncPoints;
    VkQueue myQueue;

    Semaphore* myPendingWaitSemaphores[64];
    uint myNextPendingWaitSemaphoreIdx;
  };
}


