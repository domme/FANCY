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
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        LOG_INFO("Patching resource state: Resource %s has state %s, but needs to transition to %s on the commandlist.",
          resource->myName.c_str(), RenderCore::ResourceUsageStateToString(resState.myState), RenderCore::ResourceUsageStateToString(localState.myFirstDstState));
#endif  
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

      resState.myState = localState.myState;
      resState.myQueueType = localState.myQueue;
    }

    if (numBarriers > 0)
    {
      CommandList* ctx = RenderCore::BeginCommandList(mostSpecificNeededQueue, (uint)CommandListFlags::NO_RESOURCE_STATE_TRACKING);
      for (uint i = 0u; i < numBarriers; ++i)
      {
        const BarrierData& barrier = barriers[i];
        ctx->ResourceBarrier(barrier.myResource, barrier.mySrcState, barrier.myDstState, barrier.mySrcQueue, barrier.myDstQueue);
      }

      const uint64 patchingCommandListFence = RenderCore::ExecuteAndFreeCommandList(ctx);

      if (mostSpecificNeededQueue != aCommandList->myCommandListType)
        RenderCore::GetCommandQueue(aCommandList->myCommandListType)->StallForFence(patchingCommandListFence);
    }
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
