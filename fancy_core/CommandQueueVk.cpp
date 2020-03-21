#include "fancy_core_precompile.h"
#include "CommandQueueVk.h"

#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "CommandList.h"
#include "CommandListVk.h"

namespace Fancy
{
  CommandQueueVk::CommandQueueVk(CommandListType aType)
    : CommandQueue(aType)
    , myQueue(nullptr)
    , myTimelineSemaphore(nullptr)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const RenderCore_PlatformVk::QueueInfo& queueInfo = platformVk->myQueueInfos[(uint)aType];
    ASSERT(queueInfo.myQueueFamilyIndex >= 0u && queueInfo.myQueueIndex >= 0u);

    vkGetDeviceQueue(platformVk->myDevice, queueInfo.myQueueFamilyIndex, queueInfo.myQueueIndex, &myQueue);
    ASSERT(myQueue != nullptr);

    VkSemaphoreTypeCreateInfo semTypeInfo = {};
    semTypeInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    semTypeInfo.pNext = nullptr;
    semTypeInfo.initialValue = myLastCompletedFenceVal;
    semTypeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    VkSemaphoreCreateInfo semInfo = {};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semInfo.pNext = &semTypeInfo;
    semInfo.flags = 0u;

    ASSERT_VK_RESULT(vkCreateSemaphore(platformVk->myDevice, &semInfo, nullptr, &myTimelineSemaphore));
  }
//---------------------------------------------------------------------------//
  CommandQueueVk::~CommandQueueVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    WaitForIdle();
    vkDestroySemaphore(platformVk->myDevice, myTimelineSemaphore, nullptr);
  }
//---------------------------------------------------------------------------//
  bool CommandQueueVk::IsFenceDone(uint64 aFenceVal)
  {
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    
    if (myLastCompletedFenceVal < aFenceVal)
    {
      uint64 currValue = 0ull;
      ASSERT_VK_RESULT(vkGetSemaphoreCounterValue(platformVk->myDevice, myTimelineSemaphore, &currValue));

      if (currValue > myLastCompletedFenceVal)
        myLastCompletedFenceVal = currValue;
    }

    bool done = myLastCompletedFenceVal >= aFenceVal;
    return done;
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::WaitForFence(uint64 aFenceVal)
  {
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

    if (IsFenceDone(aFenceVal))
      return;

    VkSemaphoreWaitInfo waitInfo = {};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = nullptr;
    waitInfo.flags = 0u;
    waitInfo.semaphoreCount = 1u;
    waitInfo.pSemaphores = &myTimelineSemaphore;
    waitInfo.pValues = &aFenceVal;

    ASSERT_VK_RESULT(vkWaitSemaphores(RenderCore::GetPlatformVk()->myDevice, &waitInfo, UINT64_MAX));
    myLastCompletedFenceVal = glm::max(myLastCompletedFenceVal, aFenceVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::WaitForIdle()
  {
    WaitForFence(myNextFenceVal - 1u);
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::StallForQueue(const CommandQueue* aCommandQueue)
  {
    const CommandQueueVk* otherQueue = static_cast<const CommandQueueVk*>(aCommandQueue);
    ASSERT(otherQueue != this);

    VkSemaphore otherQueueSemaphore = otherQueue->myTimelineSemaphore;
    uint64 waitVal = otherQueue->myNextFenceVal - 1u;

    for (uint i = 0u; i < myPendingStallSemaphores.Size(); ++i)
    {
      if (myPendingStallSemaphores[i].first == otherQueueSemaphore)
        myPendingStallSemaphores[i].second = glm::max(myPendingStallSemaphores[i].second, waitVal);

      return;
    }

    myPendingStallSemaphores.Add({ otherQueueSemaphore, waitVal });
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::StallForFence(uint64 aFenceVal)
  {
    const CommandQueueVk* otherQueue = static_cast<const CommandQueueVk*>(RenderCore::GetCommandQueue(aFenceVal));
    ASSERT(otherQueue != this);

    VkSemaphore otherQueueSemaphore = otherQueue->myTimelineSemaphore;
    
    for (uint i = 0u; i < myPendingStallSemaphores.Size(); ++i)
    {
      if (myPendingStallSemaphores[i].first == otherQueueSemaphore)
        myPendingStallSemaphores[i].second = glm::max(myPendingStallSemaphores[i].second, aFenceVal);

      return;
    }

    myPendingStallSemaphores.Add({ otherQueueSemaphore, aFenceVal });
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueVk::ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode)
  {
    ASSERT(aCommandList->GetType() == myType);
    ASSERT(aCommandList->IsOpen());
    aCommandList->Close();

    const VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    CommandListVk* commandListVk = static_cast<CommandListVk*>(aCommandList);
    VkCommandBuffer commandBuffer = commandListVk->GetCommandBuffer();

    StaticArray<VkSemaphore, (uint)CommandListType::NUM> waitSemaphores;
    StaticArray<uint64, (uint)CommandListType::NUM> waitValues;
    for (uint i = 0u; i < myPendingStallSemaphores.Size(); ++i)
    {
      waitSemaphores.Add(myPendingStallSemaphores[i].first);
      waitValues.Add(myPendingStallSemaphores[i].second);
    }

    uint64 fenceValPostSubmit = myNextFenceVal++;

    VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = {};
    timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineSubmitInfo.pNext = nullptr;
    timelineSubmitInfo.pWaitSemaphoreValues = waitValues.IsEmpty() ? nullptr : waitValues.GetBuffer();
    timelineSubmitInfo.signalSemaphoreValueCount = 1u;
    timelineSubmitInfo.pSignalSemaphoreValues = &fenceValPostSubmit;
    timelineSubmitInfo.waitSemaphoreValueCount = waitValues.Size();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = &timelineSubmitInfo;
    submitInfo.waitSemaphoreCount = waitSemaphores.Size();
    submitInfo.pWaitSemaphores = waitSemaphores.IsEmpty() ? nullptr : waitSemaphores.GetBuffer();
    submitInfo.pWaitDstStageMask = &stageFlags;
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1u;
    submitInfo.pSignalSemaphores = &myTimelineSemaphore;

    ASSERT_VK_RESULT(vkQueueSubmit(myQueue, 1u, &submitInfo, nullptr));

    aCommandList->PostExecute(fenceValPostSubmit);

    if (aSyncMode == SyncMode::BLOCKING)
      WaitForFence(fenceValPostSubmit);

    return fenceValPostSubmit;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueVk::ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode)
  {
    const uint64 fenceVal = ExecuteCommandListInternal(aCommandList, aSyncMode);
    aCommandList->PreBegin();
    return fenceVal;
  }
//---------------------------------------------------------------------------//
}


