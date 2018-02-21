#pragma once

#include "FancyCorePrerequisites.h"
#include "TimeManager.h"

namespace Fancy { namespace Rendering {
  class RenderingProcess;
  class RenderOutput;
} }

namespace Fancy {
  class Window;
  class Time;
  struct RenderingStartupParameters;
}

namespace Fancy {   
//---------------------------------------------------------------------------//
  class FancyRuntime
  {
  public:
    static FancyRuntime* Init(HINSTANCE anAppInstanceHandle, const RenderingStartupParameters& someParams);
    static void Shutdown();
    static FancyRuntime* GetInstance();

    void BeginFrame();
    void Update(double _dt);
    void EndFrame();

    HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }

    Time& GetRealTimeClock() { return myRealTimeClock; }
    uint64 GetCurrentFrameIndex() const { return myFrameIndex; }
    Rendering::RenderOutput* GetRenderOutput() { return myRenderOutput.get(); }
    
  private:
    explicit FancyRuntime(HINSTANCE anAppInstanceHandle);
    ~FancyRuntime();

    void DoFirstFrameTasks();

    static FancyRuntime* ourInstance;

    HINSTANCE myAppInstanceHandle;
    uint64 myFrameIndex;
    Time myRealTimeClock;
    SharedPtr<Rendering::RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy
