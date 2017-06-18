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
  class RenderWindow;
//---------------------------------------------------------------------------//
  class DLLEXPORT RenderView
  {
  public:
    RenderView(HINSTANCE anAppInstanceHandle, uint32 aRenderingTechnique, const SharedPtr<GraphicsWorld>& aWorld);
    ~RenderView();

    GraphicsWorld* GetWorld() const { return myGraphicsWorld.get(); }
    Rendering::RenderingProcess* GetRenderingProcess() const { return myRenderingProcess; }
    Rendering::RenderOutput* GetRenderOutput() const { return myRenderOutput.get(); }
    RenderWindow* GetRenderWindow() const;

    void Startup();
    void Tick(const Time& aClock);

  private:
    SharedPtr<GraphicsWorld> myGraphicsWorld;

    // TODO: All different RenderingProcesses should be kept in the Runtime-Instance instead of re-creating one for each View
    ScopedPtr<Rendering::RenderingProcess> myRenderingProcess;  

    SharedPtr<Rendering::RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
}
