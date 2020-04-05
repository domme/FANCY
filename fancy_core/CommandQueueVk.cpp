#include "fancy_core_precompile.h"
#include "CommandQueueVk.h"

#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "CommandList.h"
#include "CommandListVk.h"
#include "Texture.h"
#include "GpuBuffer.h"
#include "GpuResourceDataVk.h"

#if FANCY_ENABLE_VK

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

    ResolveResourceHazardData(aCommandList);

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
  void CommandQueueVk::ResolveResourceHazardData(CommandList* aCommandList)
  {
    DynamicArray<CommandListVk::BufferMemoryBarrierData> patchingBufferBarriers;
    DynamicArray<CommandListVk::ImageMemoryBarrierData> patchingImageBarriers;

    CommandListVk* cmdListVk = static_cast<CommandListVk*>(aCommandList);

    for (auto it : cmdListVk->myLocalHazardData)
    {
      const GpuResource* resource = it.first;

      GpuResourceHazardData& globalHazardData = resource->GetHazardData();
      const CommandListVk::LocalHazardData& localHazardData = it.second;

      DynamicArray<CommandListVk::BufferMemoryBarrierData> subresourceBufferBarriers;
      DynamicArray<CommandListVk::ImageMemoryBarrierData> subresourceImageBarriers;
      for (uint subIdx = 0u; subIdx < localHazardData.mySubresources.size(); ++subIdx)
      {
        GpuSubresourceHazardDataVk& globalSubData = globalHazardData.myVkData.mySubresources[subIdx];
        const CommandListVk::SubresourceHazardData& localSubData = localHazardData.mySubresources[subIdx];
        const SubresourceLocation subresource = resource->GetSubresourceLocation(subIdx);

        if (localSubData.myIsSharedReadState)
        {
          ASSERT(!localSubData.myWasWritten);
          globalSubData.myContext = CommandListType::SHARED_READ;
        }

        if (!localSubData.myWasUsed || (globalSubData.myAccessMask == localSubData.myFirstDstAccessFlags && globalSubData.myImageLayout == localSubData.myFirstDstImageLayout))
          continue;

        if (resource->IsTexture())
        {
          bool couldMerge = false;
          if (!subresourceImageBarriers.empty())
          {
            ASSERT(subIdx > 0);  // Otherwise subresourceImageBarriers should be empty.
            CommandListVk::ImageMemoryBarrierData& lastBarrier = subresourceImageBarriers.back();
            couldMerge = lastBarrier.mySrcAccessMask == globalSubData.myAccessMask &&
              lastBarrier.mySrcLayout == globalSubData.myImageLayout &&
              lastBarrier.myDstAccessMask == localSubData.myFirstDstAccessFlags &&
              lastBarrier.myDstLayout == localSubData.myFirstDstImageLayout &&
              resource->GetSubresourceIndex(lastBarrier.mySubresourceRange.Last()) == subIdx - 1u;

            if (couldMerge)
              lastBarrier.mySubresourceRange = SubresourceRange(lastBarrier.mySubresourceRange.First(), subresource);
          }

          if (!couldMerge)
          {
            CommandListVk::ImageMemoryBarrierData barrier;
            barrier.myImage = resource->myNativeData.To<GpuResourceDataVk*>()->myImage;
            barrier.mySrcAccessMask = globalSubData.myAccessMask;
            barrier.mySrcLayout = (VkImageLayout)globalSubData.myImageLayout;
            barrier.myDstAccessMask = localSubData.myFirstDstAccessFlags;
            barrier.myDstLayout = localSubData.myFirstDstImageLayout;
            barrier.mySubresourceRange = SubresourceRange(subresource);
            subresourceImageBarriers.push_back(barrier);
          }
        }
        else
        {
          ASSERT(subIdx == 0);
          CommandListVk::BufferMemoryBarrierData barrier;
          barrier.myBuffer = resource->myNativeData.To<GpuResourceDataVk*>()->myBuffer;
          barrier.myBufferSize = static_cast<const GpuBuffer*>(resource)->GetByteSize();
          barrier.mySrcAccessMask = globalSubData.myAccessMask;
          barrier.myDstAccessMask = localSubData.myFirstDstAccessFlags;
          subresourceBufferBarriers.push_back(barrier);
        }

        globalSubData.myAccessMask = localSubData.myAccessFlags;
        globalSubData.myImageLayout = localSubData.myImageLayout;
        if (localSubData.myWasWritten)
          globalSubData.myContext = aCommandList->GetType();
      }

      if (!subresourceBufferBarriers.empty())
        patchingBufferBarriers.insert(patchingBufferBarriers.end(), subresourceBufferBarriers.begin(), subresourceBufferBarriers.end());
      if (!subresourceImageBarriers.empty())
        patchingImageBarriers.insert(patchingImageBarriers.end(), subresourceImageBarriers.begin(), subresourceImageBarriers.end());
    }

    if (!patchingBufferBarriers.empty() || !patchingImageBarriers.empty())
    {
      CommandList* ctx = RenderCore::BeginCommandList(myType);
      CommandListVk* ctxVk = static_cast<CommandListVk*>(ctx);

      for (const CommandListVk::BufferMemoryBarrierData& barrier : patchingBufferBarriers)
        ctxVk->AddBarrier(barrier);
      for (const CommandListVk::ImageMemoryBarrierData& barrier : patchingImageBarriers)
        ctxVk->AddBarrier(barrier);

      ctxVk->FlushBarriers();
      
      ExecuteAndFreeCommandList(ctx);
    }
  }
//---------------------------------------------------------------------------//
}

#endif