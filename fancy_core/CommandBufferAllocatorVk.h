#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class CommandBufferAllocatorVk
  {
  public:
    CommandBufferAllocatorVk(CommandListType aType);
    ~CommandBufferAllocatorVk();

    VkCommandBuffer GetNewCommandBuffer();
    void ReleaseCommandBuffer(VkCommandBuffer aCommandBuffer, uint64 aCommandBufferDoneFence);

  private:
    void UpdateAvailable(VkCommandBuffer* aRequestedCommandBuffer = nullptr);

    VkCommandPool myCommandPool;
    CommandListType myCommandListType;

    std::vector<VkCommandBuffer> myCommandBufferPool;
    std::list<VkCommandBuffer> myAvailableCommandBuffers;
    std::list<std::pair<uint64, VkCommandBuffer>> myReleasedWaitingCommandBuffers;
  };
}

#endif