#pragma once

#include "FancyCoreDefines.h"
#include "TimeManager.h"
#include "Ptr.h"
#include "WindowsIncludes.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct WindowParameters;
  class Window;
  class Time;
  class RenderingProcess;
  class RenderOutput;
//---------------------------------------------------------------------------//
  class FancyRuntime
  {
  public:
    static FancyRuntime* Init(HINSTANCE anAppInstanceHandle, const char** someArguments, uint aNumArguments, const WindowParameters& someWindowParams);
    static void Shutdown();
    static FancyRuntime* GetInstance();

    void BeginFrame();
    void Update(double _dt);
    void EndFrame();

    HINSTANCE GetAppInstanceHandle() const { return myAppInstanceHandle; }

    Time& GetRealTimeClock() { return myRealTimeClock; }
    RenderOutput* GetRenderOutput() const { return myRenderOutput.get(); }
    
  private:
    explicit FancyRuntime(HINSTANCE anAppInstanceHandle);
    ~FancyRuntime();

    static FancyRuntime* ourInstance;

    HINSTANCE myAppInstanceHandle;
    Time myRealTimeClock;
    SharedPtr<RenderOutput> myRenderOutput;
  };
//---------------------------------------------------------------------------//
} // end of namespace Fancy
