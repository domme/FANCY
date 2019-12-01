#include "fancy_core_precompile.h"
#include "CommandBufferAllocatorVk.h"
#include "CommandQueue.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandBufferAllocatorVk::CommandBufferAllocatorVk(CommandListType aType)
    : myCommandPool(nullptr)
    , myCommandListType(aType)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = platformVk->myQueueInfos[(uint) aType].myQueueFamilyIndex;
    ASSERT_VK_RESULT(vkCreateCommandPool(platformVk->myDevice, &createInfo, nullptr, &myCommandPool));
  }
//---------------------------------------------------------------------------//
  CommandBufferAllocatorVk::~CommandBufferAllocatorVk()
  {
    UpdateAvailable();
    ASSERT(myAvailableCommandBuffers.size() == myCommandBufferPool.size(),
      "There are still some command buffers in flight when destroying the command buffer pool");

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    if (!myCommandBufferPool.empty())
      vkFreeCommandBuffers(platformVk->myDevice, myCommandPool, (uint)myCommandBufferPool.size(), myCommandBufferPool.data());

    vkDestroyCommandPool(platformVk->myDevice, myCommandPool, nullptr);
  }
//---------------------------------------------------------------------------//
  VkCommandBuffer CommandBufferAllocatorVk::GetNewCommandBuffer()
  {
    // Check if some of the waiting allocators can be made available again
    VkCommandBuffer commandBuffer = nullptr;

    UpdateAvailable(&commandBuffer);

    if (commandBuffer != nullptr)
      return commandBuffer;

    if (!myAvailableCommandBuffers.empty())
    {
      VkCommandBuffer buffer = myAvailableCommandBuffers.front();
      myAvailableCommandBuffers.pop_front();

      return buffer;
    }

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    VkCommandBufferAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.commandBufferCount = 1u;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = myCommandPool;
    ASSERT_VK_RESULT(vkAllocateCommandBuffers(platformVk->myDevice, &allocateInfo, &commandBuffer));

    myCommandBufferPool.push_back(commandBuffer);
    return commandBuffer;
  }
//---------------------------------------------------------------------------//
  void CommandBufferAllocatorVk::ReleaseCommandBuffer(VkCommandBuffer aCommandBuffer, uint64 aCommandBufferDoneFence)
  {
#if defined (FANCY_RENDERER_USE_VALIDATION)
    for (VkCommandBuffer buffer : myAvailableCommandBuffers)
      ASSERT(buffer != aCommandBuffer);

    for (auto waitingBufferEntry : myReleasedWaitingCommandBuffers)
      ASSERT(waitingBufferEntry.second != aCommandBuffer);
#endif // FANCY_RENDERSYSTEM_USE_VALIDATION

    myReleasedWaitingCommandBuffers.push_back(std::make_pair(aCommandBufferDoneFence, aCommandBuffer));
  }
//---------------------------------------------------------------------------//
  void CommandBufferAllocatorVk::UpdateAvailable(VkCommandBuffer* aRequestedCommandBuffer /* = nullptr */)
  {
    bool foundBuffer = false;
    auto it = myReleasedWaitingCommandBuffers.begin();
    for (; it != myReleasedWaitingCommandBuffers.end(); )
    {
      const uint64 waitingFenceVal = it->first;
      VkCommandBuffer buffer = it->second;
      CommandQueue* queue = RenderCore::GetCommandQueue(myCommandListType);

      if (queue->IsFenceDone(waitingFenceVal))
      {
        if (waitingFenceVal != 0)
          ASSERT_VK_RESULT(vkResetCommandBuffer(buffer, 0u));

        it = myReleasedWaitingCommandBuffers.erase(it);

        if (aRequestedCommandBuffer != nullptr && !foundBuffer)
        {
          *aRequestedCommandBuffer = buffer;
          foundBuffer = true;
        }
        else
        {
          myAvailableCommandBuffers.push_back(buffer);
        }
      }
      else
        ++it;
    }
  }
//---------------------------------------------------------------------------//
}
