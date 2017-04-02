#pragma once

#include "RendererPrerequisites.h"
#include "CommandListType.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class CommandContext
  {
  public:
    CommandContext(CommandListType aType);
    virtual ~CommandContext() {}

    CommandListType GetType() const { return myCommandListType; }

    virtual void Reset() = 0;
    virtual uint64 ExecuteAndReset(bool aWaitForCompletion = false) = 0;

  protected:
    CommandListType myCommandListType;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class CommandContextPool
  {
  public:
    static CommandContext* AllocateContext(CommandListType aType);
    static void FreeContext(CommandContext* aContext);
  };
//---------------------------------------------------------------------------//
} }
