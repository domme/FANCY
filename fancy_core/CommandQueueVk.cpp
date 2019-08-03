#include "fancy_core_precompile.h"
#include "CommandQueueVk.h"

#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

namespace Fancy
{
  CommandQueueVk::CommandQueueVk(CommandListType aType)
    : CommandQueue(aType)
    , myQueue(nullptr)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkGetDeviceQueue(platformVk->myDevice, platformVk->myQueueIndices[(uint)aType], 0u, &myQueue);
    ASSERT(myQueue != nullptr);
  }

  CommandQueueVk::~CommandQueueVk()
  {
    
  }

  bool CommandQueueVk::IsFenceDone(uint64 aFenceVal)
  {
    ASSERT(false, "Not implemented");
    return true;
  }

  uint64 CommandQueueVk::SignalAndIncrementFence()
  {
    ASSERT(false, "Not implemented");
    return 0u;
  }

  void CommandQueueVk::WaitForFence(uint64 aFenceVal)
  {
    ASSERT(false, "Not implemented");
  }

  void CommandQueueVk::WaitForIdle()
  {
    ASSERT(false, "Not implemented");
  }

  void CommandQueueVk::StallForQueue(const CommandQueue* aCommandQueue)
  {
    ASSERT(false, "Not implemented");
  }

  void CommandQueueVk::StallForFence(uint64 aFenceVal)
  {
    ASSERT(false, "Not implemented");
  }

  uint64 CommandQueueVk::ExecuteCommandListInternal(CommandList* aContext, SyncMode aSyncMode)
  {
    ASSERT(false, "Not implemented");
    return 0u;
  }

  uint64 CommandQueueVk::ExecuteAndResetCommandListInternal(CommandList* aContext, SyncMode aSyncMode)
  {
    ASSERT(false, "Not implemented");
    return 0u;
  }
}

