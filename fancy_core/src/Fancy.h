#pragma once

#include "FancyCorePrerequisites.h"
#include "TimeManager.h"

namespace Fancy { namespace Scene {
class SceneNode;
class Scene;
  typedef std::shared_ptr<Scene> ScenePtr;
} }

namespace Fancy { namespace Rendering {
  class RenderingProcess;
  class RenderOutput;
} }

namespace Fancy {
  class RenderWindow;
  class RenderView;
  class Time;
  class GraphicsWorld;
  struct EngineParameters;
}

namespace Fancy {   
//---------------------------------------------------------------------------//
  class FancyRuntime
  {
  public:
    static FancyRuntime* Init(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams);
    static void Shutdown();
    static FancyRuntime* GetInstance();

    void BeginFrame();
    void Update(double _dt);
    void EndFrame();

    HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }

    Time& GetRealTimeClock() { return myRealTimeClock; }
    uint64 GetCurrentFrameIndex() const { return myFrameIndex; }

    RenderWindow* GetMainRenderWindow() const;
    RenderView* GetMainView() const { return myMainView.get(); }
    GraphicsWorld* GetMainWorld() const { return myMainWorld.get(); }

  private:
    explicit FancyRuntime(HINSTANCE anAppInstanceHandle);
    ~FancyRuntime();

    void Internal_Init(const EngineParameters& someParams);

    void DoFirstFrameTasks();

    static FancyRuntime* ourInstance;

    HINSTANCE myAppInstanceHandle;
    uint64 myFrameIndex;

    // TODO: Add support for secondary views
    SharedPtr<GraphicsWorld> myMainWorld;
    std::unique_ptr<RenderView> myMainView;

    Time myRealTimeClock;
    std::vector<RenderView*> myViews;
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy
