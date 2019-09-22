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
  CommandList* CommandQueue::BeginCommandList()
  {
    if (!myAvailableCommandLists.empty())
    {
      CommandList* commandList = myAvailableCommandLists.front();
      if (!commandList->IsOpen())
        commandList->Reset();
      myAvailableCommandLists.pop_front();

      return commandList;
    }

    myCommandListPool.push_back(std::unique_ptr<CommandList>(RenderCore::GetPlatform()->CreateCommandList(myType)));
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
    const uint64 fence = ExecuteCommandListInternal(aCommandList, aSyncMode);
    FreeCommandList(aCommandList);
    return fence;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    aCommandList->FlushBarriers();
    return ExecuteAndResetCommandListInternal(aCommandList, aSyncMode);
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
