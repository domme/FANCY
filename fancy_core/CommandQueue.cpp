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

    // This method will launch auxillary command lists to transition any resources to the state needed upon the first use in the command list.
    // Cross-queue transitions from a more restricted to a less restricted queue need to be resolved by executing a patching command list on the less restricted queue.
    // However, read-to-read transitions don't need to be treated that way. For those transitions it should be fine to just transition from/to whatever the current queue type can understand.
    // But for Write-Read and Read-Write transitions it needs to be done this way.

    for (uint iRes = 0u; iRes < numTrackedResources; ++iRes)
    {
      const GpuResource* resource = aCommandList->myTrackedResources[iRes];
      const CommandList::ResourceStateTracking& cmdListResState = aCommandList->myResourceStateTrackings[iRes];
      const GpuResourceStateTracking& resState = resource->myStateTracking;

      const uint numSubresources = resource->myNumSubresources;
      ASSERT(numSubresources == (uint)cmdListResState.mySubresources.size());

      for (uint iSub = 0u; iSub < numSubresources; ++iSub)
      {
        const CommandList::SubresourceStateTracking& cmdListSubState = cmdListResState.mySubresources[iSub];
        if (cmdListSubState.myState == GpuResourceUsageState::UNKNOWN)  // Hasn't been modified in aCommandList
          continue;

        const GpuSubresourceStateTracking& subState = resState.mySubresources[iSub];


        // TODO: Continue here
      }
    }


    CommandList* ctx = BeginCommandList((uint) CommandListFlags::NO_RESOURCE_STATE_TRACKING);

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
