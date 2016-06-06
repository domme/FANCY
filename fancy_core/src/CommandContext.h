#pragma once

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_COMMANDCONTEXT

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class CommandContext : public PLATFORM_DEPENDENT_NAME(CommandContext)
  {
  public:
    CommandContext(CommandListType aType) 
      : PLATFORM_DEPENDENT_NAME(CommandContext)(aType) 
    {

    }

    static CommandContext* AllocateContext(CommandListType aType);
    static void FreeContext(CommandContext* aContext);
  };
//---------------------------------------------------------------------------//
} }
