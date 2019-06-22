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
    if (aCommandList->myNumTrackedResources == 0)
      return;

    // TODO: It might be necessary to add more complexity to the hazard tracking so that we can also detect scenarios where the patching command list has to perform cross-queue transitions
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

        GpuResourceUsageState& currState = resource->myHazardData.mySubresourceStates[iSub];
        
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
          ctx->SubresourceBarrier(&resource, &subresourceList, &numSubresources, &currState, &subTracking.myFirstDstState, 1u, srcQueue, dstQueue);
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
