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
#include <fancy_core/Profiling.h>
#include <fancy_core/MathUtil.h>

#include "ProfilerWindow.h"

using namespace Fancy;

ProfilerWindow profilerWindow;

bool show_test_window = false;
bool show_profiler_window = true;

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;

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

  ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);
}

void Update()
{
  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
  myRuntime->Update(0.016f);

  if (ImGui::Button("Test Window")) show_test_window ^= 1;
  if (ImGui::Button("Profiler Window")) show_profiler_window ^= 1;

  // if (ImGui::Checkbox("Pause", &profilerWindow.myIsPaused))
  //   Profiling::SetPause(profilerWindow.myIsPaused);

  ImGui::SliderFloat("Scale", &profilerWindow.myScale, 0.1f, 10.0f);

  // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
  if (show_test_window)
  {
    ImGui::ShowTestWindow(&show_test_window);
  }

  if (show_profiler_window)
    profilerWindow.Render();
}

void Render()
{
  PROFILE_FUNCTION();

  LongDummyFunc();

  CommandQueue* queue = RenderCore::GetCommandQueue(CommandListType::Graphics);
  CommandContext* ctx = RenderCore::AllocateContext(CommandListType::Graphics);

  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
  queue->ExecuteContext(ctx);
  RenderCore::FreeContext(ctx);

  ImGui::Render();
  myRuntime->EndFrame();
}

void Shutdown()
{
  ImGuiRendering::Shutdown();
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