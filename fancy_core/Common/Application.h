#pragma once

#include "Camera.h"
#include "CameraController.h"
#include "TimeManager.h"
#include "Common/Input.h"
#include "Common/FancyCoreDefines.h"
#include "EASTL/string.h"
#include "Common/Ptr.h"

struct ImGuiContext;

namespace Fancy {
  class Assets;
  class Window;
  class RenderOutput;
  struct RenderPlatformProperties;
  struct WindowParameters;

  class Application
  {
  public:
    Application(HINSTANCE anInstanceHandle,
      const char** someArguments,
      uint aNumArguments,
      const char* aName,
      const char* aRelativeRootFolder,
      const RenderPlatformProperties& someRenderProperties,
      const WindowParameters& someWindowParams);

    virtual ~Application();

    virtual void OnWindowResized(uint aWidth, uint aHeight);
    virtual void BeginFrame();
    virtual void Update();
    virtual void Render();
    virtual void EndFrame();

    const char* GetName() const { return myName.c_str(); }

  protected:
    HINSTANCE myAppInstanceHandle;
    eastl::string myName;
    CameraController myCameraController;
    SharedPtr<Time> myRealTimeClock;
    InputState myInputState;
    Camera myCamera;
    SharedPtr<RenderOutput> myRenderOutput;
  };

}
