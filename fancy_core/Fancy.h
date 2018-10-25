#pragma once

#include "FancyCorePrerequisites.h"
#include "TimeManager.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct WindowParameters;
  class Window;
  class Time;
  struct RenderingStartupParameters;
  class RenderingProcess;
  class RenderOutput;
//---------------------------------------------------------------------------//
  class FancyRuntime
  {
  public:
    static FancyRuntime* Init(HINSTANCE anAppInstanceHandle, const RenderingStartupParameters& someParams, const WindowParameters& someWindowParams);
    static void Shutdown();
    static FancyRuntime* GetInstance();

    void BeginFrame();
    void Update(double _dt);
    void EndFrame();

    HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }

    Time& GetRealTimeClock() { return myRealTimeClock; }
    uint64 GetCurrentFrameIndex() const { return myFrameIndex; }
    RenderOutput* GetRenderOutput() const { return myRenderOutput.get(); }
    
  private:
    explicit FancyRuntime(HINSTANCE anAppInstanceHandle);
    ~FancyRuntime();

    void DoFirstFrameTasks();

    static FancyRuntime* ourInstance;

    HINSTANCE myAppInstanceHandle;
    uint64 myFrameIndex;
    Time myRealTimeClock;
    SharedPtr<RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy
