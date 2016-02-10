#ifndef INCLUDE_RENDERER_H
#define INCLUDE_RENDERER_H

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_RENDERER

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class Renderer : public PLATFORM_DEPENDENT_NAME(Renderer)
  {
    public:
      Renderer(void* aNativeWindowHandle) : PLATFORM_DEPENDENT_NAME(Renderer)(aNativeWindowHandle) {}
      virtual ~Renderer() {}

    protected:
      
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  class RenderingSubsystem : public PLATFORM_DEPENDENT_NAME(RenderingSubsystem)
  {
  public:
    /// Init platform-independent stuff
    static void Init();
    /// Shutdown platform-independent stuff
    static void Shutdown();

  private:
    RenderingSubsystem() {}
  };
//---------------------------------------------------------------------------//
} // end of namespace Rendering
} // end of namespace Fancy

#endif  // INCLUDE_RENDERER_H