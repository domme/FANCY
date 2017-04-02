#ifndef INCLUDE_ENGINECOMMON_H
#define INCLUDE_ENGINECOMMON_H

#include "FancyCorePrerequisites.h"
#include "ScopedPtr.h"
#include "TimeManager.h"
#include "RenderCore_Platform.h"

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
}

namespace Fancy {   
//---------------------------------------------------------------------------//
  enum class RenderingTechnique
  {
    FORWARD = 0,
    FORWARD_PLUS,

    NUM
  };
//---------------------------------------------------------------------------//
  struct DLLEXPORT EngineParameters
  {
    EngineParameters() 
      : myResourceFolder("../../../resources/")
      , myRenderingTechnique(RenderingTechnique::FORWARD) 
      , myRenderingApi(Rendering::RenderingApi::DX12)
    { }

    String myResourceFolder;
    RenderingTechnique myRenderingTechnique;
    Rendering::RenderingApi myRenderingApi;
  };
//---------------------------------------------------------------------------//
  class DLLEXPORT FancyRuntime
  {
  public:
    static FancyRuntime* Init(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams);
    static FancyRuntime* GetInstance();

    void Update(double _dt);

    HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }

    Time& GetRealTimeClock() { return myRealTimeClock; }
    uint64 GetCurrentFrameIndex() const { return myFrameIndex; }

    RenderView* GetMainView() const { return myMainView; }
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
    ScopedPtr<RenderView> myMainView;

    Time myRealTimeClock;
    std::vector<RenderView*> myViews;
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
