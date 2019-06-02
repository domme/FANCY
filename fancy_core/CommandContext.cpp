#include "fancy_core_precompile.h"
#include "CommandContext.h"

#include "RenderCore.h"
#include "CommandList.h"
#include "CommandQueue.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandContext::CommandContext(CommandListType aType)
    : myCommandList(RenderCore::AllocateCommandList(aType))
  {

  }
//---------------------------------------------------------------------------//
  CommandContext::~CommandContext()
  {
    RenderCore::FreeCommandList(myCommandList);
  }
//---------------------------------------------------------------------------//
  uint64 CommandContext::Execute(SyncMode aSyncMode)
  {
    return RenderCore::GetCommandQueue(myCommandList->GetType())->ExecuteCommandList(myCommandList, aSyncMode);
  }
//---------------------------------------------------------------------------//
  uint64 CommandContext::ExecuteAndReset(SyncMode aSyncMode /* = SyncMode::ASYNC*/)
  {
    return RenderCore::GetCommandQueue(myCommandList->GetType())->ExecuteAndResetCommandList(myCommandList, aSyncMode);
  }
//---------------------------------------------------------------------------//
}
