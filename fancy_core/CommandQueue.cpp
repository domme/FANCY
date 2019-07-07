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
    ResolveResourceBarriers(aCommandList);
    const uint64 fence = ExecuteCommandListInternal(aCommandList, aSyncMode);
    FreeCommandList(aCommandList);
    return fence;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    aCommandList->FlushBarriers();
    ResolveResourceBarriers(aCommandList);
    return ExecuteAndResetCommandListInternal(aCommandList, aSyncMode);
  }
//---------------------------------------------------------------------------//
  void CommandQueue::ResolveResourceBarriers(CommandList* aCommandList)
  {
    const uint numTrackedResources = aCommandList->myNumTrackedResources;
    if (numTrackedResources == 0u)
      return;

    struct BarrierData
    {
      const GpuResource* myResource;
      CommandListType mySrcQueue;
      GpuResourceUsageState mySrcState;
      CommandListType myDstQueue;
      GpuResourceUsageState myDstState;
    };
    BarrierData* barriers = (BarrierData*)alloca(sizeof(BarrierData) * numTrackedResources);
    uint numBarriers = 0u;
    CommandListType mostSpecificNeededQueue = aCommandList->myCommandListType;

    for (uint iRes = 0; iRes < aCommandList->myNumTrackedResources; ++iRes)
    {
      const GpuResource* resource = aCommandList->myTrackedResources[iRes];
      GpuResourceStateTracking& resState = resource->myStateTracking;
      const CommandList::ResourceStateTracking& localState = aCommandList->myResourceStateTrackings[iRes];

      if (localState.myFirstSrcState != GpuResourceUsageState::UNKNOWN)
        ASSERT(localState.myFirstSrcState == resState.myState);
      if (localState.myFirstSrcQueue != CommandListType::UNKNOWN)
        ASSERT(localState.myFirstSrcQueue == resState.myQueueType);

      if (GpuResourceStateTracking::IsBarrierNeeded(resState.myQueueType, resState.myState, localState.myFirstDstQueue, localState.myFirstDstState))
      {
        BarrierData& barrier = barriers[numBarriers++];
        barrier.myResource = resource;
        barrier.mySrcQueue = resState.myQueueType;
        barrier.mySrcState = resState.myState;
        barrier.myDstQueue = localState.myFirstDstQueue;
        barrier.myDstState = localState.myFirstDstState;

        while (!GpuResourceStateTracking::QueueCanTransitionFrom(mostSpecificNeededQueue, resState.myQueueType, resState.myState))
        {
          ASSERT(mostSpecificNeededQueue > (CommandListType) 0);
          mostSpecificNeededQueue = static_cast<CommandListType>(static_cast<uint>(mostSpecificNeededQueue) - 1);
        }
      }
      else
      {
        // This is a read-read transition that doesn't need a barrier. Keep the more generic state
        resState.myQueueType = resState.myQueueType < localState.myFirstDstQueue ? resState.myQueueType : localState.myFirstDstQueue;
        resState.myState = 
      }
      


      needsHigherLevelQueue |= resource->myStateTracking.myQueueType < aCommandList->myCommandListType;
    }
    

    CommandList* ctx = nullptr;
    if (needsHigherLevelQueue)
      ctx = RenderCore::BeginCommandList(CommandListType::Graphics, (uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING);
    else
      ctx = BeginCommandList((uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING);

    for (uint iRes = 0; iRes < aCommandList->myNumTrackedResources; ++iRes)
    {
      const GpuResource* resource = aCommandList->myTrackedResources[iRes];
      const CommandList::ResourceStateTracking& localState = aCommandList->myResourceStateTrackings[iRes];
      GpuResourceStateTracking& resState = resource->myStateTracking;

    }


    


    CommandList* ctx = 

    for (uint iRes = 0; iRes < aCommandList->myNumTrackedResources; ++iRes)
    {
      const GpuResource* resource = aCommandList->myTrackedResources[iRes];
      const CommandList::ResourceStateTracking& resTracking = aCommandList->myResourceStateTrackings[iRes];



      ASSERT(resource->myNumSubresources == (uint) resTracking.mySubresources.size());
      for (uint iSub = 0u; iSub < resource->myNumSubresources; ++iSub)
      {
        const CommandList::SubresourceStateTracking& subTracking = resTracking.mySubresources[iSub];
        if (subTracking.myState == GpuResourceUsageState::UNKNOWN)  // Hasn't been modified in aCommandList
          continue;

        GpuSubresourceStateTracking& currSubTracking = resource->myStateTracking.mySubresources[iSub];
        GpuResourceUsageState& currState = currSubTracking.myState;
        
        if (subTracking.myFirstSrcState == GpuResourceUsageState::UNKNOWN && currState != subTracking.myFirstDstState) // Needs a patching barrier
        {
          const uint16 subresources[] = { (uint16)iSub };
          const uint16* subresourceList = subresources;
          const uint numSubresources = 1u;
          CommandListType srcQueue = ctx->GetType();  // TODO: Deal with cross-queue cases
          CommandListType dstQueue = ctx->GetType();
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
          LOG_INFO("Patching resource state: Resource %s (subresource %d) has state %s, but needs to transition to %s on the commandlist.",
            resource->myName.c_str(), iSub, RenderCore::ResourceUsageStateToString(currState), RenderCore::ResourceUsageStateToString(subTracking.myFirstDstState));
#endif  
          ctx->SubresourceBarrier(resource, subresourceList, numSubresources, currState, subTracking.myFirstDstState, srcQueue, dstQueue);
        }
        else
        {
          ASSERT(subTracking.myFirstSrcState == currState);
        }

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        LOG_INFO("New state for resource %s (subresource %d) after commandList: %s",
          resource->myName.c_str(), iSub, RenderCore::ResourceUsageStateToString(subTracking.myState));
#endif  
        currState = subTracking.myState;
      }
    }

    ctx->FlushBarriers();
    ExecuteAndFreeCommandList(ctx);
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
