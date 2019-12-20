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

      RecreateSemaphores(syncPoint.mySemaphore);
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
      for (uint i = mySyncPoints.Size() - 1u; syncPoint == nullptr && i >= 0; --i)
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
      StallForFence(otherQueue->mySyncPoints.GetLast().myWaitingOnVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::StallForFence(uint64 aFenceVal)
  {
    const CommandQueueVk* otherQueue = static_cast<const CommandQueueVk*>(RenderCore::GetCommandQueue(aFenceVal));
    ASSERT(otherQueue != this);

    if (!otherQueue->mySyncPoints.IsEmpty())
    {
      // Find the corresponding syncPoint in the other queue
      SyncPoint* syncPoint = nullptr;
      for (uint i = otherQueue->mySyncPoints.Size() - 1u; syncPoint != nullptr && i != 0; --i)
      {
        if (otherQueue->mySyncPoints[i].myWaitingOnVal == aFenceVal)
          syncPoint = &otherQueue->mySyncPoints[i];
      }

      if (syncPoint != nullptr && !syncPoint->mySemaphore.myWasWaitedOn[(uint)otherQueue->myType])
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
      ASSERT(!semaphore->myWasWaitedOn[(uint)myType]);  // Should early-out in stallForFence
        
      waitSemaphores[numValidWaitSemaphores++] = semaphore->mySemaphores[(uint)myType];
      semaphore->myWasWaitedOn[(uint)myType] = true;
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

    aCommandList->PostExecute(fenceVal);

    if (aSyncMode == SyncMode::BLOCKING)
      WaitForFence(fenceVal);

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueVk::ExecuteAndResetCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode)
  {
    const uint64 fenceVal = ExecuteCommandListInternal(aCommandList, aSyncMode);
    aCommandList->PreBegin();
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
    
    RecreateSemaphores(syncPoint.mySemaphore);

    return &syncPoint;
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::AddPendingWaitSemaphore(Semaphore* aWaitSemaphore)
  {
    for (uint i = 0; i < myNextPendingWaitSemaphoreIdx; ++i)
      if (myPendingWaitSemaphores[i] == aWaitSemaphore)
        return;

    ASSERT(myNextPendingWaitSemaphoreIdx < ARRAY_LENGTH(myPendingWaitSemaphores) - 1u);
    myPendingWaitSemaphores[myNextPendingWaitSemaphoreIdx++] = aWaitSemaphore;
  }
//---------------------------------------------------------------------------//
  void CommandQueueVk::RecreateSemaphores(Semaphore& aSemaphore)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0u;
    for (uint iType = 0u; iType < (uint)CommandListType::NUM; ++iType)
    {
      if (aSemaphore.mySemaphores[iType] != nullptr && !aSemaphore.myWasWaitedOn[iType])
      {
        // This semaphore is still signaled and needs to be recreated since the only way to reset semaphores is to wait on them on a command list submission
        vkDestroySemaphore(platformVk->myDevice, aSemaphore.mySemaphores[iType], nullptr);
      }

      ASSERT_VK_RESULT(vkCreateSemaphore(platformVk->myDevice, &semaphoreCreateInfo, nullptr, &aSemaphore.mySemaphores[iType]));

      aSemaphore.myWasWaitedOn[iType] = false;
    }
  }
//---------------------------------------------------------------------------//
}

