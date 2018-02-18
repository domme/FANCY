#pragma once

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
  class RenderView
  {
  public:
    RenderView(HINSTANCE anAppInstanceHandle, uint aRenderingTechnique, const SharedPtr<GraphicsWorld>& aWorld);
    ~RenderView();

    GraphicsWorld* GetWorld() const { return myGraphicsWorld.get(); }
    Rendering::RenderingProcess* GetRenderingProcess() const { return myRenderingProcess; }
    Rendering::RenderOutput* GetRenderOutput() const { return myRenderOutput.get(); }
    RenderWindow* GetRenderWindow() const;

    void Startup() const;
    void BeginFrame() const;
    void Tick(const Time& aClock) const;
    void EndFrame() const;

  private:
    SharedPtr<GraphicsWorld> myGraphicsWorld;

    // TODO: All different RenderingProcesses should be kept in the Runtime-Instance instead of re-creating one for each View
    Rendering::RenderingProcess* myRenderingProcess;  

    std::unique_ptr<Rendering::RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
}
