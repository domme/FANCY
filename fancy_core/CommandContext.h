#pragma once

#include "RenderEnums.h"

namespace Fancy
{
 //---------------------------------------------------------------------------//
  class CommandList;
//---------------------------------------------------------------------------//
  /*
   * A scoped wrapper around a CommandList that handles allocating, freeing and executing of CommandLists from RenderCore
   * internally.
   */
  class CommandContext
  {
  public:
    CommandContext(CommandListType aType);
    ~CommandContext();

    CommandList* GetCommandList() const { return myCommandList; }
    CommandList* operator->() const { return myCommandList; }
    CommandList& operator*() const { return *myCommandList; }
  
    uint64 Execute(SyncMode aSyncMode = SyncMode::ASYNC);
    uint64 ExecuteAndReset(SyncMode aSyncMode = SyncMode::ASYNC);

    private:
      CommandList* myCommandList;
  };
//---------------------------------------------------------------------------//
}

