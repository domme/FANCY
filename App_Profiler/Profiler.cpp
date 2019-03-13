#include <windows.h>
#include <fancy_imgui/imgui.h>
#include <fancy_imgui/imgui_impl_fancy.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/RendererPrerequisites.h>
#include "fancy_core/CommandContext.h"
#include <fancy_core/Fancy.h>
#include <fancy_core/Window.h>
#include <fancy_core/CommandQueue.h>
#include <fancy_core/Profiler.h>
#include <fancy_core/MathUtil.h>
#include <fancy_core/Input.h>

#include "ProfilerWindow.h"
#include <array>

using namespace Fancy;

ProfilerWindow profilerWindow;

bool show_test_window = false;
bool show_profiler_window = true;

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
InputState myInputState;
ImGuiContext* myImGuiContext = nullptr;

void DummyFunc3()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);
}

void LongDummyFunc2()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 99999)
    MathUtil::hash_combine(hash, i++);

  DummyFunc3();
}

void LongDummyFunc()
{
  PROFILE_FUNCTION();

  int i = 0;
  while (i < 99999)
    ++i;

  LongDummyFunc2();
}

void Func0()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);
}

void Func1_0()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);
}

void Func1_1()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);
}

void Func1()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);

  Func1_0();
  Func1_1();
}

void Func2()
{
  PROFILE_FUNCTION();

  uint64 hash = 0u;
  int i = 0;
  while (i < 999)
    MathUtil::hash_combine(hash, i++);
}

void OnWindowResized(uint aWidth, uint aHeight)
{

}

void Init(HINSTANCE anInstanceHandle)
{
  RenderingStartupParameters params;
  params.myRenderingTechnique = RenderingTechnique::FORWARD;

  Fancy::WindowParameters windowParams;
  windowParams.myWidth = 1920;
  windowParams.myHeight = 1080;
  windowParams.myTitle = "Profiler window example";

  myRuntime = FancyRuntime::Init(anInstanceHandle, params, windowParams);

  myRenderOutput = myRuntime->GetRenderOutput();
  myWindow = myRenderOutput->GetWindow();

  std::function<void(uint, uint)> onWindowResized = &OnWindowResized;
  myWindow->myOnResize.Connect(onWindowResized);
  myWindow->myWindowEventHandler.Connect(&myInputState, &InputState::OnWindowEvent);

  myImGuiContext = ImGui::CreateContext();
  ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);
}

void Update()
{
  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
  myRuntime->Update(0.016f);

  Func0();
  Func1();
  Func2();

  // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
  if (myInputState.myKeyState['z'] && !myInputState.myLastKeyState['z'])
    show_profiler_window = !show_profiler_window;

  if (show_profiler_window)
    profilerWindow.Render();

  if (ImGui::Button("Test Window")) show_test_window ^= 1;

  if (show_test_window)
  {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
    ImGui::ShowDemoWindow(&show_test_window);
  }
}

void Render()
{
  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);

  GPU_BEGIN_PROFILE(ctx, "ClearRenderTarget", 0u);
  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
  GPU_END_PROFILE(ctx);

  queue->ExecuteContext(ctx);
  RenderCore::FreeContext(ctx);

  ImGui::Render();
  myRuntime->EndFrame();
}

void Shutdown()
{
  ImGuiRendering::Shutdown();
  ImGui::DestroyContext(myImGuiContext);
  myImGuiContext = nullptr;
  FancyRuntime::Shutdown();
  myRuntime = nullptr;
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
  Init(hInstance);

  MSG msg = { 0 };
  while (true)
  {
    // Process any messages in the queue.
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);

      if (msg.message == WM_QUIT)
        break;
    }

    Update();
    Render();
  }

  Shutdown();

  return 0;
}