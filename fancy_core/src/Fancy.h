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
      static FancyRuntime* GetInstance() { return ourInstance; }

      void Update(double _dt);

      HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }
      const Time& GetRealTimeClock() const { return myRealTimeClock; }
      uint64 GetCurrentFrameIndex() const { return myFrameIndex; }

    private:
      FancyRuntime(HINSTANCE anAppInstanceHandle, const EngineParameters& someParams);
      ~FancyRuntime();

      void DoFirstFrameTasks();

      static FancyRuntime* ourInstance;

      HINSTANCE myAppInstanceHandle;
      uint64 myFrameIndex;
      // TODO: Add support for secondary views
      ScopedPtr<RenderView> myDefaultView;

      Time myRealTimeClock;
    };
//---------------------------------------------------------------------------//
} // end of namespace Fancy

#endif  // INCLUDE_ENGINECOMMON_H
