#include "fancy_core_precompile.h"
#include "Application.h"

#include "Fancy.h"
#include "Window.h"
#include "Debug/Profiler.h"
#include "Rendering/CommandList.h"
#include "Rendering/RenderCore.h"
#include "Rendering/RenderOutput.h"

namespace Fancy
{
  Fancy::Application::Application(HINSTANCE anInstanceHandle, const char** someArguments, uint aNumArguments,
    const char* aName, const char* aRelativeRootFolder, const RenderPlatformProperties& someRenderProperties, const WindowParameters& someWindowParams)
    : myWindow(nullptr)
    , myRenderOutput(nullptr)
    , myName(aName)
    , myCameraController(&myCamera)
  {
    myRuntime = FancyRuntime::Init(anInstanceHandle, someArguments, aNumArguments, someWindowParams, someRenderProperties, aRelativeRootFolder);

    myRenderOutput = myRuntime->GetRenderOutput();
    myWindow = myRenderOutput->GetWindow();

    myWindow->myOnResize.Connect(this, &Application::OnWindowResized);
    myWindow->myWindowEventHandler.Connect(&myInputState, &InputState::OnWindowEvent);
  }

  Application::~Application()
  {
    FancyRuntime::Shutdown();
  }

  void Application::OnWindowResized(uint aWidth, uint aHeight)
  {
    myCamera.myWidth = myWindow->GetWidth();
    myCamera.myHeight = myWindow->GetHeight();
    myCamera.UpdateProjection();
  }

  void Application::BeginFrame()
  {
    myRuntime->BeginFrame();
  }

  void Application::Update()
  {
    myCameraController.Update(0.016f, myInputState);
    myRuntime->Update(0.016);
  }

  void Application::Render()
  {
    CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
    GPU_BEGIN_PROFILE(ctx, "ClearRenderTarget", 0u);
    float clearColor[] = { 0.3f, 0.3f, 0.3f, 0.0f };
    ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
    GPU_END_PROFILE(ctx);
    RenderCore::ExecuteAndFreeCommandList(ctx);
  }

  void Application::EndFrame()
  {
    myRuntime->EndFrame();
  }
}

