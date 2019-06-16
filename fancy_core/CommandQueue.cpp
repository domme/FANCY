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
    , myAllocatedCommandListStack{ nullptr }
    , myNextFreeCmdListStackIdx(0u)
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

      ASSERT(myNextFreeCmdListStackIdx < ARRAY_LENGTH(myAllocatedCommandListStack));
      myAllocatedCommandListStack[myNextFreeCmdListStackIdx++] = commandList;

      return commandList;
    }

    myCommandListPool.push_back(std::unique_ptr<CommandList>(RenderCore::GetPlatform()->CreateContext(myType, someCommandListFlags)));
    CommandList* commandList = myCommandListPool.back().get();

    return commandList;
  }
//---------------------------------------------------------------------------//
  void CommandQueue::FreeCommandList(CommandList* aCommandList)
  {
    ASSERT(myNextFreeCmdListStackIdx > 0 && aCommandList == myAllocatedCommandListStack[myNextFreeCmdListStackIdx -1]);
    ASSERT(std::find(myAvailableCommandLists.begin(), myAvailableCommandLists.end(), aCommandList) == myAvailableCommandLists.end());

    myAllocatedCommandListStack[--myNextFreeCmdListStackIdx] = nullptr;
    myAvailableCommandLists.push_back(aCommandList);
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndFreeCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    ASSERT(myNextFreeCmdListStackIdx > 0 && aCommandList == myAllocatedCommandListStack[myNextFreeCmdListStackIdx-1]);
    const uint64 fence = ExecuteCommandListInternal(aCommandList, aSyncMode);
    FreeCommandList(aCommandList);
    return fence;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueue::ExecuteAndResetCommandList(CommandList* aCommandList, SyncMode aSyncMode)
  {
    ASSERT(myNextFreeCmdListStackIdx > 0 && aCommandList == myAllocatedCommandListStack[myNextFreeCmdListStackIdx - 1]);
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
