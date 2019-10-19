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
    , myPendingWaitSemaphores{ nullptr }
    , myNextPendingWaitSemaphoreIdx(0u)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    const RenderCore_PlatformVk::QueueInfo& queueInfo = platformVk->myQueueInfos[(uint)aType];
    ASSERT(queueInfo.myQueueFamilyIndex >= 0u && queueInfo.myQueueIndex >= 0u);

    vkGetDeviceQueue(platformVk->myDevice, queueInfo.myQueueFamilyIndex, queueInfo.myQueueIndex, &myQueue);
    ASSERT(myQueue != nullptr);

    // Initialize the fence-pool
    SyncPoint* syncPoints = mySyncPoints.GetBuffer();
    for (uint i = 0u, e = mySyncPoints.Capacity(); i < e; ++i)
    {
      SyncPoint& syncPoint = syncPoints[i];
      syncPoint.myWaitingOnVal = myLastCompletedFenceVal;

      VkFenceCreateInfo fenceCreateInfo;
      fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceCreateInfo.pNext = nullptr;
      fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
      ASSERT_VK_RESULT(vkCreateFence(platformVk->myDevice, &fenceCreateInfo, nullptr, &syncPoint.myFence));

      VkSemaphoreCreateInfo semaphoreCreateInfo;
      semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      semaphoreCreateInfo.pNext = nullptr;
      semaphoreCreateInfo.flags = 0u;
      for (uint iType = 0u; iType < (uint) CommandListType::NUM; ++iType)
      {
        ASSERT_VK_RESULT(vkCreateSemaphore(platformVk->myDevice, &semaphoreCreateInfo, nullptr, &syncPoint.mySemaphore.mySemaphores[iType]));
        syncPoint.mySemaphore.myIsPending[iType] = false;
      }
    }
  }
//---------------------------------------------------------------------------//
  CommandQueueVk::~CommandQueueVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    SyncPoint* syncPoints = mySyncPoints.GetBuffer();
    for (uint i = 0u, e = mySyncPoints.Capacity(); i < e; ++i)
    {
      SyncPoint& syncPoint = syncPoints[i];

      for (uint iType = 0u; iType < (uint)CommandListType::NUM; ++iType)
        vkDestroySemaphore(platformVk->myDevice, syncPoint.mySemaphore.mySemaphores[iType], nullptr);

      vkDestroyFence(platformVk->myDevice, syncPoint.myFence, nullptr);
    }
  }
//---------------------------------------------------------------------------//
  bool CommandQueueVk::IsFenceDone(uint64 aFenceVal)
  {
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    // If we can't tell that a fence-value is passed from the last completed val, we have to update the cache by checking 
    // our newest pending fences for completion
    if (myLastCompletedFenceVal < aFenceVal && !mySyncPoints.IsEmpty())
    {
      for (uint i = mySyncPoints.Size() - 1; i != 0; --i)
      {
        SyncPoint& syncPoint = mySyncPoints[i];
        if (vkGetFenceStatus(platformVk->myDevice, syncPoint.myFence) == VK_SUCCESS)
        {
          myLastCompletedFenceVal = glm::max(syncPoint.myWaitingOnVal, myLastCompletedFenceVal);
          break;
        }
      }
    }

    return myLastCompletedFenceVal >= aFenceVal;
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::WaitForFence(uint64 aFenceVal)
  {
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

    if (IsFenceDone(aFenceVal))
      return;

    SyncPoint* syncPoint = nullptr;
    if (!mySyncPoints.IsEmpty())
    {
      for (uint i = mySyncPoints.Size() - 1u; syncPoint == nullptr && i != 0; --i)
      {
        if (mySyncPoints[i].myWaitingOnVal == aFenceVal)
          syncPoint = &mySyncPoints[i];
      }
    }
    
    ASSERT(syncPoint); // SyncPoint is only nullptr if aFenceVal is so old that we don't have it in the mySyncPoints-cache anymore. In this case, IsFenceDone() should've returned true already though...

    // Wait indefinitely for the fence to become signalled
    while (vkWaitForFences(RenderCore::GetPlatformVk()->myDevice, 1u, &syncPoint->myFence, true, UINT64_MAX) != VK_SUCCESS)
    {

    }

    myLastCompletedFenceVal = glm::max(myLastCompletedFenceVal, syncPoint->myWaitingOnVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::WaitForIdle()
  {
    if (!mySyncPoints.IsEmpty())
      WaitForFence(myNextFenceVal - 1u);
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::StallForQueue(const CommandQueue* aCommandQueue)
  {
    const CommandQueueVk* otherQueue = static_cast<const CommandQueueVk*>(aCommandQueue);
    ASSERT(otherQueue != this);
    
    if (!otherQueue->mySyncPoints.IsEmpty())
      AddPendingWaitSemaphore(&otherQueue->mySyncPoints.GetLast().mySemaphore);
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::StallForFence(uint64 aFenceVal)
  {
    const CommandQueueVk* otherQueue = static_cast<const CommandQueueVk*>(RenderCore::GetCommandQueue(aFenceVal));
    ASSERT(otherQueue != this);

    if (!otherQueue->mySyncPoints.IsEmpty())
    {
      // Find the corresponding fence in the other queue
      SyncPoint* syncPoint = nullptr;
      for (uint i = otherQueue->mySyncPoints.Size() - 1u; syncPoint != nullptr && i != 0; --i)
      {
        if (otherQueue->mySyncPoints[i].myWaitingOnVal == aFenceVal)
          syncPoint = &otherQueue->mySyncPoints[i];
      }

      if (syncPoint != nullptr)
        AddPendingWaitSemaphore(&syncPoint->mySemaphore);

      // If the fence isn't found it should only mean that the fenceVal is too old and is not included in the fences-array anymore.
      // That should also mean that the fence is already passed by the CPU and the stall can be safely ignored.
    }
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueVk::ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode)
  {
    ASSERT(aCommandList->GetType() == myType);
    ASSERT(aCommandList->IsOpen());
    aCommandList->Close();

    const uint numWaitSemaphores = myNextPendingWaitSemaphoreIdx;
    uint numValidWaitSemaphores = 0u;
    VkSemaphore* waitSemaphores = (VkSemaphore*) alloca(sizeof(VkSemaphore) * numWaitSemaphores);
    for (uint i = 0; i < numWaitSemaphores; ++i)
    {
      Semaphore* semaphore = myPendingWaitSemaphores[i];
      if (semaphore->myIsPending[(uint)myType])
      {
        waitSemaphores[numValidWaitSemaphores++] = semaphore->mySemaphores[(uint)myType];
        semaphore->myIsPending[(uint)myType] = false;
      }
    }

    SyncPoint* commandListDoneSyncPoint = GetNewSyncPoint();

    const VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    
    CommandListVk* commandListVk = static_cast<CommandListVk*>(aCommandList);
    VkCommandBuffer commandBuffer = commandListVk->GetCommandBuffer();

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = numValidWaitSemaphores;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = &stageFlags;
    submitInfo.commandBufferCount = 1u;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = (uint)CommandListType::NUM;
    submitInfo.pSignalSemaphores = commandListDoneSyncPoint->mySemaphore.mySemaphores;

    ASSERT_VK_RESULT(vkQueueSubmit(myQueue, 1u, &submitInfo, commandListDoneSyncPoint->myFence));
    const uint64 fenceVal = commandListDoneSyncPoint->myWaitingOnVal;

    aCommandList->ReleaseGpuResources(fenceVal);

    if (aSyncMode == SyncMode::BLOCKING)
      WaitForFence(fenceVal);

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueVk::ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode)
  {
    const uint64 fenceVal = ExecuteCommandListInternal(aCommandList, aSyncMode);
    aCommandList->Reset();
    return fenceVal;
  }
//---------------------------------------------------------------------------//
  CommandQueueVk::SyncPoint* CommandQueueVk::GetNewSyncPoint()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    // Get a new fence from the pool
    if (mySyncPoints.IsFull())
    {
      SyncPoint& firstFence = mySyncPoints[0];
      ASSERT(vkGetFenceStatus(platformVk->myDevice, firstFence.myFence) == VK_SUCCESS, "CommandQueueVk needs to reuse an existing fence but it is not passed yet. Do we need more fences in the pool?");
      mySyncPoints.RemoveFirstElement();
    }

    SyncPoint& syncPoint = mySyncPoints.Add();
    syncPoint.myWaitingOnVal = myNextFenceVal++;
    ASSERT_VK_RESULT(vkResetFences(platformVk->myDevice, 1u, &syncPoint.myFence));

    for (uint i = 0u; i < (uint)CommandListType::NUM; ++i)
      syncPoint.mySemaphore.myIsPending[i] = true;

    return &syncPoint;
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::AddPendingWaitSemaphore(Semaphore* aWaitSemaphore)
  {
    ASSERT(myNextPendingWaitSemaphoreIdx < ARRAY_LENGTH(myPendingWaitSemaphores) - 1u);
    myPendingWaitSemaphores[myNextPendingWaitSemaphoreIdx++] = aWaitSemaphore;
  }
//---------------------------------------------------------------------------//
}

