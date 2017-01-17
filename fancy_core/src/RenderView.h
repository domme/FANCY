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
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderView
  {
  public:
    RenderView(HINSTANCE anAppInstanceHandle, uint32 aRenderingTechnique);
    ~RenderView();

  private:
    ScopedPtr<GraphicsWorld> myGraphicsWorld;
    ScopedPtr<Rendering::RenderingProcess> myRenderingProcess;
    ScopedPtr<Rendering::RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
}
