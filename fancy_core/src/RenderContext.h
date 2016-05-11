#pragma once

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_RENDERCONTEXT

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class RenderContext : public PLATFORM_DEPENDENT_NAME(RenderContext)
  {
  public:
    RenderContext() {}
    
    static RenderContext* AllocateContext();
    static void FreeContext(RenderContext* aContext);
  };
//---------------------------------------------------------------------------//
} }