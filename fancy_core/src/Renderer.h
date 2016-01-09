#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_RENDERER

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class Renderer : public PLATFORM_DEPENDENT_NAME(Renderer)
  {
    public:
      static Renderer& getInstance() { static Renderer instance; return instance; }
      virtual ~Renderer() {}

    protected:
      Renderer() {}
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class RenderingSubsystem : public PLATFORM_DEPENDENT_NAME(RenderingSubsystem)
  {
  
  };
//---------------------------------------------------------------------------//
} // end of namespace Rendering
} // end of namespace Fancy

#endif  // INCLUDE_RENDERER_H