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

  protected:
    struct Semaphore
    {
      VkSemaphore mySemaphores[(uint)CommandListType::NUM];
      bool myIsPending[(uint)CommandListType::NUM];
    };

    struct SyncPoint
    {
      Semaphore mySemaphore;
      VkFence myFence;
      uint64 myWaitingOnVal;
    };

    uint64 ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode) override;
    uint64 ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode) override;

    SyncPoint* GetNewSyncPoint();
    void AddPendingWaitSemaphore(Semaphore* aWaitSemaphore);

    mutable StaticCircularArray<SyncPoint, 256> mySyncPoints;
    VkQueue myQueue;

    Semaphore* myPendingWaitSemaphores[64];
    uint myNextPendingWaitSemaphoreIdx;
  };
}


