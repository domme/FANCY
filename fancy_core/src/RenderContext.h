#pragma once

#include "RendererPrerequisites.h"
#include "CommandContext.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class RenderContext : public CommandContext
  {
  public:
    RenderContext();
    virtual ~RenderContext() {}
  
  };
//---------------------------------------------------------------------------//
} }
