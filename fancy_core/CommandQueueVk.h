#pragma once
#include "CommandQueue.h"

#include "VkPrerequisites.h"

namespace Fancy
{
  class CommandQueueVk final : public CommandQueue
  {
  public:
    CommandQueueVk(CommandListType aType);
    virtual ~CommandQueueVk();

    bool IsFenceDone(uint64 aFenceVal) override;
    uint64 SignalAndIncrementFence() override;
    void WaitForFence(uint64 aFenceVal) override;
    void WaitForIdle() override;
    void StallForQueue(const CommandQueue* aCommandQueue) override;
    void StallForFence(uint64 aFenceVal) override;

  protected:
    uint64 ExecuteCommandListInternal(CommandList* aContext, SyncMode aSyncMode) override;
    uint64 ExecuteAndResetCommandListInternal(CommandList* aContext, SyncMode aSyncMode) override;

    VkQueue myQueue;
  };
}


