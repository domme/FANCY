#pragma once
#include "ScopedPtr.h"

namespace Fancy { namespace Scene {
  class Scene;
} }

namespace Fancy { namespace Rendering {
  class RenderOutput;
  class RenderingProcess;
} }

namespace Fancy {
//---------------------------------------------------------------------------//
  class GraphicsWorld;
  class Time;
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderView
  {
  public:
    RenderView(HINSTANCE anAppInstanceHandle, uint32 aRenderingTechnique, const SharedPtr<GraphicsWorld>& aWorld);
    ~RenderView();

    void Startup();
    void Tick(const Time& aClock);

  private:
    SharedPtr<GraphicsWorld> myGraphicsWorld;
    ScopedPtr<Rendering::RenderingProcess> myRenderingProcess;
    ScopedPtr<Rendering::RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
}
