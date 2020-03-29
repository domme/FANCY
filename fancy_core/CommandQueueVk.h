#pragma once
#include "CommandQueue.h"
#include "StaticArray.h"

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

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
    uint64 ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode) override;
    uint64 ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode) override;
    
    StaticArray<std::pair<VkSemaphore, uint64>, (uint)CommandListType::NUM> myPendingStallSemaphores;  // Semaphores from other queues that the next submit needs to wait on
    VkSemaphore myTimelineSemaphore;
    VkQueue myQueue;
  };
}

#endif