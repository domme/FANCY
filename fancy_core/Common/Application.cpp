#include "fancy_core_precompile.h"
#include "Application.h"

#include "CommandLine.h"
#include "Window.h"
#include "Debug/Profiler.h"
#include "IO/AssetManager.h"
#include "IO/PathService.h"
#include "Rendering/CommandList.h"
#include "Rendering/CommandQueue.h"
#include "Rendering/RenderCore.h"
#include "Rendering/RenderOutput.h"

namespace Fancy
{
  Fancy::Application::Application(HINSTANCE anInstanceHandle, const char** someArguments, uint aNumArguments,
    const char* aName, const char* aRelativeRootFolder, const RenderPlatformProperties& someRenderProperties, const WindowParameters& someWindowParams)
    : myAppInstanceHandle(anInstanceHandle)
    , myName(aName)
    , myCameraController(&myCamera)
    , myRealTimeClock(new Time)
  {
    CommandLine::CreateInstance(someArguments, aNumArguments);

    // Init IO-subsystem
    Path::InitRootFolder(aRelativeRootFolder);

    // Init rendering subsystem
    if (!RenderCore::IsInitialized())
      RenderCore::Init(someRenderProperties, myRealTimeClock);

    ASSERT(RenderCore::IsInitialized());

    myRenderOutput = RenderCore::CreateRenderOutput(anInstanceHandle, someWindowParams);
    myRenderOutput->GetWindow()->myOnResize.Connect(this, &Application::OnWindowResized);
    myRenderOutput->GetWindow()->myWindowEventHandler.Connect(&myInputState, &InputState::OnWindowEvent);

    myAssetManager.reset(new AssetManager());
  }

  Application::~Application()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);
  }

  void Application::OnWindowResized(uint aWidth, uint aHeight)
  {
    myCamera.myWidth = aWidth;
    myCamera.myHeight = aHeight;
    myCamera.UpdateProjection();
  }

  void Application::BeginFrame()
  {
    Profiler::BeginFrame();
    RenderCore::BeginFrame();
    Profiler::BeginFrameGPU();
    myRenderOutput->BeginFrame();
  }

  void Application::Update()
  {
    myCameraController.Update(0.016f, myInputState);
    myRealTimeClock->Update(0.016f);
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
    myRenderOutput->EndFrame();
    Profiler::EndFrameGPU();
    RenderCore::EndFrame();
    Profiler::EndFrame();
    ++Time::ourFrameIdx;
  }
}

