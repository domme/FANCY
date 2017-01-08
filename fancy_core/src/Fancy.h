#ifndef INCLUDE_ENGINECOMMON_H
#define INCLUDE_ENGINECOMMON_H

#include "FancyCorePrerequisites.h"
#include "ScopedPtr.h"
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
    struct EngineParameters
    {
      EngineParameters() 
        : myResourceFolder("../../../resources/")
        , myRenderingTechnique(RenderingTechnique::FORWARD) 
      { }

      String myResourceFolder;
      RenderingTechnique myRenderingTechnique;
    };
//---------------------------------------------------------------------------//
    class DLLEXPORT FancyRuntime
    {
    public:
      static FancyRuntime* Init(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams);
      static FancyRuntime* GetInstance();

      void Update(double _dt);

      HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }

      // TODO: There can be more than one RenderOutput/RenderWindow in the future
      RenderWindow* GetCurrentRenderWindow();
      Rendering::RenderOutput* GetCurrentRenderOutput() { return myRenderOutput.Get(); }

      Rendering::RenderingProcess* GetRenderingProcess() { return myRenderingProcess.Get(); }
      Scene::Scene* GetCurrentScene() { return myScene.Get(); }
      Time& GetRealTimeClock() { return myRealTimeClock; }
      uint64 GetCurrentFrameIndex() { return myFrameIndex; }

      Scene::SceneNode* Import(const std::string& aPath);

    private:
      FancyRuntime(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams);
      ~FancyRuntime();

      void DoFirstFrameTasks();

      HINSTANCE myAppInstanceHandle;
      
      // TODO: Where to put these?
      Time myRealTimeClock;
      uint64 myFrameIndex;


      
      ScopedPtr<GraphicsWorld> myGraphicsWorld;
      
      ScopedPtr<Rendering::RenderingProcess> myRenderingProcess;
      ScopedPtr<Rendering::RenderOutput> myRenderOutput;
    };

//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
