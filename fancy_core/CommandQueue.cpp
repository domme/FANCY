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
    ResolveResourceBarriers(aCommandList);
    const uint64 fence = ExecuteCommandListInternal(aCommandList, aSyncMode);
    FreeCommandList(aCommandList);
    return fence;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
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
      const CommandList::ResourceHazardEntry& entry = aCommandList->myResourceStateTrackings[iRes];

      ASSERT(resource->myNumSubresources == (uint) entry.mySubresourceStates.size());
      ASSERT(resource->myNumSubresources == (uint) entry.myFirstSubresourceStates.size());
      for (uint iSub = 0u; iSub < resource->myNumSubresources; ++iSub)
      {
        const GpuResourceUsageState firstNewState = entry.myFirstSubresourceStates[iSub];
        if (firstNewState == GpuResourceUsageState::UNKNOWN)  // Hasn't been modified in aCommandList
          continue;

        GpuResourceUsageState& currState = resource->myHazardData.mySubresourceStates[iSub];
        if (currState != firstNewState) // Needs a patching barrier
        {
          const uint16 subresources[] = { (uint16)iSub };
          const uint16* subresourceList = subresources;
          const uint numSubresources = 1u;
          CommandListType srcQueue = ctx->GetType();  // TODO: Deal with cross-queue cases
          CommandListType dstQueue = ctx->GetType();
          ctx->SubresourceBarrier(&resource, &subresourceList, &numSubresources, &currState, &firstNewState, 1u, srcQueue, dstQueue);
        }

        const GpuResourceUsageState lastNewState = entry.mySubresourceStates[iSub];
        currState = lastNewState;
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
