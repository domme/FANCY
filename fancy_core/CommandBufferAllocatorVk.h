#pragma once

#include "VkPrerequisites.h"

#include "EASTL/vector.h"
#include "EASTL/fixed_list.h"

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

    eastl::vector<VkCommandBuffer> myCommandBufferPool;
    eastl::fixed_list<VkCommandBuffer, 128> myAvailableCommandBuffers;
    eastl::fixed_list<std::pair<uint64, VkCommandBuffer>, 128> myReleasedWaitingCommandBuffers;
  };
}

#endif