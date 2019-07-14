#include "fancy_core_precompile.h"
#include "CommandQueue.h"
#include "CommandList.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandQueue::CommandQueue(CommandListType aType)
    : myType(aType)
    , myLastCompletedFenceVal((((uint64)aType) << 61ULL))
    , myNextFenceVal(myLastCompletedFenceVal + 1u)
  {
  }
//---------------------------------------------------------------------------//
  CommandQueue::~CommandQueue()
  {
    ASSERT(myCommandListPool.size() == myAvailableCommandLists.size(), "There are still some commandLists in flight");
  }
//---------------------------------------------------------------------------//
  CommandList* CommandQueue::BeginCommandList(uint someCommandListFlags)
  {
    if (!myAvailableCommandLists.empty())
    {
      CommandList* commandList = myAvailableCommandLists.front();
      if (!commandList->IsOpen())
        commandList->Reset(someCommandListFlags);
      myAvailableCommandLists.pop_front();

      return commandList;
    }

    myCommandListPool.push_back(std::unique_ptr<CommandList>(RenderCore::GetPlatform()->CreateContext(myType, someCommandListFlags)));
    CommandList* commandList = myCommandListPool.back().get();

    return commandList;
  }
//---------------------------------------------------------------------------//
  void CommandQueue::FreeCommandList(CommandList* aCommandList)
  {
    ASSERT(std::find(myAvailableCommandLists.begin(), myAvailableCommandLists.end(), aCommandList) == myAvailableCommandLists.end());
    myAvailableCommandLists.push_back(aCommandList);
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndFreeCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    aCommandList->FlushBarriers();
    return InsertCommandListTrackResourceStates(aCommandList, false, aSyncMode);
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    aCommandList->FlushBarriers();
    return InsertCommandListTrackResourceStates(aCommandList, true, aSyncMode);
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::InsertCommandListTrackResourceStates(CommandList* aCommandList, bool aReset, SyncMode aSyncMode)
  {
    const uint numTrackedResources = aCommandList->myNumTrackedResources;
    if (numTrackedResources == 0)
    {
      if (aReset)
        return ExecuteAndResetCommandListInternal(aCommandList, aSyncMode);

      const uint64 fence = ExecuteCommandListInternal(aCommandList, aSyncMode);
      FreeCommandList(aCommandList);
      return fence;
    }

    struct BarrierData
    {
      const GpuResource* myResource;
      CommandListType mySrcQueue;
      GpuResourceUsageState mySrcState;
      CommandListType myDstQueue;
      GpuResourceUsageState myDstState;
    };
    BarrierData* prePatchingBarriers = (BarrierData*)alloca(sizeof(BarrierData) * numTrackedResources);
    uint numPrePatchingBarriers = 0u;
    CommandListType prePatchingQueueType = aCommandList->myCommandListType;

    BarrierData* postPatchingBarriers = (BarrierData*)alloca(sizeof(BarrierData) * numTrackedResources);
    uint numPostPatchingBarriers = 0u;
    CommandListType postPatchingQueueType = aCommandList->myCommandListType;

    for (uint iRes = 0; iRes < aCommandList->myNumTrackedResources; ++iRes)
    {
      const GpuResource* resource = aCommandList->myTrackedResources[iRes];
      GpuResourceStateTracking& resState = resource->myStateTracking;
      const CommandList::ResourceStateTracking& localState = aCommandList->myResourceStateTrackings[iRes];

      if (localState.myFirstDstQueue == resState.myQueueType && localState.myFirstDstState == resState.myState)
        continue;

      ASSERT(localState.myFirstSrcState == GpuResourceUsageState::UNKNOWN || localState.myFirstSrcState == resState.myState);
      ASSERT(localState.myFirstSrcQueue == CommandListType::UNKNOWN || localState.myFirstSrcQueue == resState.myQueueType);

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
      LOG_INFO("Pre-patching resource state: Resource %s is on (%s-%s) but is needed as (%s-%s) on the commandList",
        resource->myName.c_str(), RenderCore::CommandListTypeToString(resState.myQueueType), RenderCore::ResourceUsageStateToString(resState.myState), 
        RenderCore::CommandListTypeToString(localState.myFirstDstQueue), RenderCore::ResourceUsageStateToString(localState.myFirstDstState));
#endif
      BarrierData& prePatchBarrier = prePatchingBarriers[numPrePatchingBarriers++];
      prePatchBarrier.myResource = resource;
      prePatchBarrier.mySrcQueue = resState.myQueueType;
      prePatchBarrier.mySrcState = resState.myState;
      prePatchBarrier.myDstQueue = localState.myFirstDstQueue;
      prePatchBarrier.myDstState = localState.myFirstDstState;

      while (!GpuResourceStateTracking::QueueUnderstandsState(prePatchingQueueType, resState.myQueueType, resState.myState))
      {
        ASSERT(prePatchingQueueType > (CommandListType) 0);
        prePatchingQueueType = static_cast<CommandListType>(static_cast<uint>(prePatchingQueueType) - 1);
      }

      resState.myState = localState.myState;
      resState.myQueueType = localState.myQueue;

      if (localState.myPendingLastDstQueue != CommandListType::UNKNOWN || localState.myPendingLastDstState != GpuResourceUsageState::UNKNOWN)
      {
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        LOG_INFO("Post-patching resource state: Resource %s needs to be transitioned to (%s-%s), but commandList of type %s doesn't understand that",
          resource->myName.c_str(), RenderCore::CommandListTypeToString(localState.myPendingLastDstQueue), RenderCore::ResourceUsageStateToString(localState.myPendingLastDstState),
          RenderCore::CommandListTypeToString(aCommandList->myCommandListType));
#endif
        BarrierData& postPatchBarrier = postPatchingBarriers[numPostPatchingBarriers++];
        postPatchBarrier.myResource = resource;
        postPatchBarrier.mySrcQueue = localState.myQueue;
        postPatchBarrier.mySrcState = localState.myState;
        postPatchBarrier.myDstQueue = localState.myPendingLastDstQueue;
        postPatchBarrier.myDstState = localState.myPendingLastDstState;

        while (!GpuResourceStateTracking::QueueUnderstandsState(postPatchingQueueType, localState.myPendingLastDstQueue, localState.myPendingLastDstState))
        {
          ASSERT(postPatchingQueueType > (CommandListType)0);
          postPatchingQueueType = static_cast<CommandListType>(static_cast<uint>(postPatchingQueueType) - 1);
        }

        resState.myState = localState.myPendingLastDstState;
        resState.myQueueType = localState.myPendingLastDstQueue;
      }
    }

    if (numPrePatchingBarriers > 0)
    {
      CommandList* ctx = RenderCore::BeginCommandList(prePatchingQueueType, (uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING);
      for (uint i = 0u; i < numPrePatchingBarriers; ++i)
      {
        const BarrierData& barrier = prePatchingBarriers[i];
        ctx->ResourceBarrier(barrier.myResource, barrier.mySrcState, barrier.myDstState, barrier.mySrcQueue, barrier.myDstQueue);
      }

      const uint64 patchingCommandListFence = RenderCore::ExecuteAndFreeCommandList(ctx);

      if (prePatchingQueueType != aCommandList->myCommandListType)
      {
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        LOG_INFO("Some resources on command-list type %s need pre-patching with command-list type %s. The queues will be serialized",
          RenderCore::CommandListTypeToString(aCommandList->myCommandListType), RenderCore::CommandListTypeToString(prePatchingQueueType));
#endif
        RenderCore::GetCommandQueue(aCommandList->myCommandListType)->StallForFence(patchingCommandListFence);
      }
    }

    uint64 fence = 0u;
    if (aReset)
    {
      fence = ExecuteAndResetCommandListInternal(aCommandList, aSyncMode);
    }
    else
    {
      fence = ExecuteCommandListInternal(aCommandList, aSyncMode);
      FreeCommandList(aCommandList);
    }
    
    if (numPostPatchingBarriers)


  }
//---------------------------------------------------------------------------//
  CommandListType CommandQueue::GetCommandListType(uint64 aFenceVal)
  {
    uint listTypeVal = aFenceVal >> 61;
    ASSERT(listTypeVal < (uint)CommandListType::NUM);

    return (CommandListType)listTypeVal;
  }
//---------------------------------------------------------------------------//
}
