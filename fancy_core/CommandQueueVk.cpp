#include "fancy_core_precompile.h"
#include "CommandQueueVk.h"

#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "CommandList.h"
#include "CommandListVk.h"
#include "Texture.h"
#include "GpuBuffer.h"
#include "GpuResourceDataVk.h"
#include "DebugUtilsVk.h"

#if FANCY_ENABLE_VK

// The validation layer reports false-positive errors when resetting commandpools when timeline semaphores are used.
// This define will always wait after each submit to not stall the output with meaningless errors.
#define FANCY_VK_WAIT_AFTER_EACH_SUBMIT 0

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
    myPendingStallSemaphores.ClearDiscard();

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

    if (RenderCore::ourDebugWaitAfterEachSubmit)
      vkQueueWaitIdle(myQueue);
    
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

      GpuResourceHazardDataVk& globalHazardData = resource->GetVkData()->myHazardData;
      const CommandListVk::LocalHazardData& localHazardData = it.second;

      DynamicArray<CommandListVk::BufferMemoryBarrierData> subresourceBufferBarriers;
      DynamicArray<CommandListVk::ImageMemoryBarrierData> subresourceImageBarriers;
      for (uint subIdx = 0u; subIdx < localHazardData.mySubresources.Size(); ++subIdx)
      {
        GpuSubresourceHazardDataVk& globalSubData = globalHazardData.mySubresources[subIdx];
        const CommandListVk::SubresourceHazardData& localSubData = localHazardData.mySubresources[subIdx];

        if (!localSubData.myWasUsed)
          continue;

        const SubresourceLocation subresource = resource->GetSubresourceLocation(subIdx);

        const VkAccessFlags oldGlobalAccessMask = globalSubData.myAccessMask;
        const VkImageLayout oldGlobalImageLayout = (VkImageLayout) globalSubData.myImageLayout;

        if (localSubData.myIsSharedReadState)
        {
          ASSERT(!localSubData.myWasWritten);
          globalSubData.myContext = CommandListType::SHARED_READ;
        }

        globalSubData.myAccessMask = localSubData.myAccessFlags;
        globalSubData.myImageLayout = localSubData.myImageLayout;
        if (localSubData.myWasWritten)
          globalSubData.myContext = aCommandList->GetType();

        if (oldGlobalAccessMask == localSubData.myFirstDstAccessFlags && oldGlobalImageLayout == localSubData.myFirstDstImageLayout)
        {
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
          if (RenderCore::ourDebugLogResourceBarriers)
          {
            if (resource->IsBuffer())
              LOG_DEBUG("Patching open transition for buffer %s: No transition needed (global/local access mask == local access mask == %s)", resource->GetName(), 
                DebugUtilsVk::AccessMaskToString(oldGlobalAccessMask).c_str());
            else
              LOG_DEBUG("Patching open transition for image %s (subresource %d): No transition needed (global/local access mask == %s // global/local image layout %s)", resource->GetName(),
                subIdx, DebugUtilsVk::AccessMaskToString(oldGlobalAccessMask).c_str(), DebugUtilsVk::ImageLayoutToString(oldGlobalImageLayout).c_str());
          }
#endif
          continue;
        }

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        if (RenderCore::ourDebugLogResourceBarriers)
        {
          if (resource->IsBuffer())
            LOG_DEBUG("Patching open transition for buffer %s: %s -> %s", resource->GetName(),
              DebugUtilsVk::AccessMaskToString(oldGlobalAccessMask).c_str(), DebugUtilsVk::AccessMaskToString(localSubData.myFirstDstAccessFlags).c_str());
          else
            LOG_DEBUG("Patching open transition for image %s (subresource %d): %s -> %s / %s -> %s", resource->GetName(),
              subIdx, DebugUtilsVk::AccessMaskToString(oldGlobalAccessMask).c_str(), DebugUtilsVk::AccessMaskToString(localSubData.myFirstDstAccessFlags).c_str(),
              DebugUtilsVk::ImageLayoutToString(oldGlobalImageLayout).c_str(), DebugUtilsVk::ImageLayoutToString(localSubData.myFirstDstImageLayout).c_str());
        }
#endif

        if (resource->IsTexture())
        {
          bool couldMerge = false;
          if (!subresourceImageBarriers.empty())
          {
            ASSERT(subIdx > 0);  // Otherwise subresourceImageBarriers should be empty.
            CommandListVk::ImageMemoryBarrierData& lastBarrier = subresourceImageBarriers.back();
            couldMerge = lastBarrier.mySrcAccessMask == oldGlobalAccessMask &&
              lastBarrier.mySrcLayout == oldGlobalImageLayout &&
              lastBarrier.myDstAccessMask == localSubData.myFirstDstAccessFlags &&
              lastBarrier.myDstLayout == localSubData.myFirstDstImageLayout &&
              resource->GetSubresourceIndex(lastBarrier.mySubresourceRange.Last()) == subIdx - 1u;

            if (couldMerge)
              lastBarrier.mySubresourceRange = SubresourceRange(lastBarrier.mySubresourceRange.First(), subresource);
          }

          if (!couldMerge)
          {
            CommandListVk::ImageMemoryBarrierData barrier;
            barrier.myImage = resource->myNativeData.To<GpuResourceDataVk>().myImage;
            barrier.myFormat = static_cast<const Texture*>(resource)->GetProperties().myFormat;
            barrier.mySrcAccessMask = oldGlobalAccessMask;
            barrier.mySrcLayout = oldGlobalImageLayout;
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
          barrier.myBuffer = resource->myNativeData.To<GpuResourceDataVk>().myBuffer;
          barrier.myBufferSize = static_cast<const GpuBuffer*>(resource)->GetByteSize();
          barrier.mySrcAccessMask = oldGlobalAccessMask;
          barrier.myDstAccessMask = localSubData.myFirstDstAccessFlags;
          subresourceBufferBarriers.push_back(barrier);
        }
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