#pragma once

#include "Camera.h"
#include "CameraController.h"
#include "Common/Input.h"
#include "Common/FancyCoreDefines.h"
#include "EASTL/string.h"

struct ImGuiContext;

namespace Fancy {
  class Window;
  class FancyRuntime;
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
    FancyRuntime* myRuntime;
    Window* myWindow;
    RenderOutput* myRenderOutput;
    eastl::string myName;
    InputState myInputState;
    Camera myCamera;
    CameraController myCameraController;
  };

}
