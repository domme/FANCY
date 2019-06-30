#include <windows.h>
#include <fancy_imgui/imgui.h>
#include <fancy_imgui/imgui_impl_fancy.h>
#include <fancy_core/RenderOutput.h>
#include <fancy_core/RenderCore.h>
#include <fancy_core/RendererPrerequisites.h>
#include "fancy_core/CommandList.h"
#include <fancy_core/Fancy.h>
#include <fancy_core/Window.h>
#include <fancy_core/CommandQueue.h>
#include <fancy_core/Profiler.h>
#include <fancy_core/Input.h>

#include <array>
#include "Test.h"
#include "Test_Profiler.h"
#include "Test_ImGui.h"
#include "Test_GpuMemoryAllocator.h"
#include "Test_Synchronization.h"
#include "Test_AsyncCompute.h"


using namespace Fancy;

FancyRuntime* myRuntime = nullptr;
Window* myWindow = nullptr;
RenderOutput* myRenderOutput = nullptr;
InputState myInputState;
ImGuiContext* myImGuiContext = nullptr;

bool test_profiler = false;
bool test_imgui = false;
bool test_gpuMemoryAllocs = false;
bool test_sychronization = false;
bool test_asyncCompute = false;

DynamicArray<UniquePtr<Test>> myTests;

void OnWindowResized(uint aWidth, uint aHeight)
{

}

void Init(HINSTANCE anInstanceHandle)
{
  RenderingStartupParameters params;
  params.myRenderingTechnique = RenderingTechnique::FORWARD;

  Fancy::WindowParameters windowParams;
  windowParams.myWidth = 1280;
  windowParams.myHeight = 720;
  windowParams.myTitle = "Fancy Engine Tests";
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

  if (ImGui::Checkbox("Test Profiler", &test_profiler))
  {
    if (!test_profiler)
      myTests.erase(std::find_if(myTests.begin(), myTests.end(), [](const UniquePtr<Test>& aTestItem) { return dynamic_cast<Test_Profiler*>(aTestItem.get()) != nullptr; }));
    else
      myTests.push_back(std::make_unique<Test_Profiler>(myRuntime, myWindow, myRenderOutput, &myInputState));
  }
  if (ImGui::Checkbox("Test ImGui", &test_imgui))
  {
    if (!test_imgui)
      myTests.erase(std::find_if(myTests.begin(), myTests.end(), [](const UniquePtr<Test>& aTestItem) { return dynamic_cast<Test_ImGui*>(aTestItem.get()) != nullptr; }));
    else
      myTests.push_back(std::make_unique<Test_ImGui>(myRuntime, myWindow, myRenderOutput, &myInputState));
  }
  if (ImGui::Checkbox("Test Gpu Memory Allocations", &test_gpuMemoryAllocs))
  {
    if (!test_gpuMemoryAllocs)
      myTests.erase(std::find_if(myTests.begin(), myTests.end(), [](const UniquePtr<Test>& aTestItem) { return dynamic_cast<Test_GpuMemoryAllocator*>(aTestItem.get()) != nullptr; }));
    else
      myTests.push_back(std::make_unique<Test_GpuMemoryAllocator>(myRuntime, myWindow, myRenderOutput, &myInputState));
  }
  if (ImGui::Checkbox("Test Synchronization", &test_sychronization))
  {
    if (!test_sychronization)
      myTests.erase(std::find_if(myTests.begin(), myTests.end(), [](const UniquePtr<Test>& aTestItem) { return dynamic_cast<Test_Synchronization*>(aTestItem.get()) != nullptr; }));
    else
      myTests.push_back(std::make_unique<Test_Synchronization>(myRuntime, myWindow, myRenderOutput, &myInputState));
  }
  if (ImGui::Checkbox("Test Async Compute", &test_asyncCompute))
  {
    if (!test_asyncCompute)
      myTests.erase(std::find_if(myTests.begin(), myTests.end(), [](const UniquePtr<Test>& aTestItem) { return dynamic_cast<Test_AsyncCompute*>(aTestItem.get()) != nullptr; }));
    else
      myTests.push_back(std::make_unique<Test_AsyncCompute>(myRuntime, myWindow, myRenderOutput, &myInputState));
  }

  ImGui::Separator();

  for (UniquePtr<Test>& testItem : myTests)
  {
    if (ImGui::TreeNode(testItem->GetName()))
    {
      testItem->OnUpdate(true);
      ImGui::TreePop();
    }
    else
    {
      testItem->OnUpdate(false);
    }
  }
}

void Render()
{
  CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
  GPU_BEGIN_PROFILE(ctx, "ClearRenderTarget", 0u);
  float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  ctx->ResourceBarrier(myRenderOutput->GetBackbuffer(), GpuResourceUsageState::READ_PRESENT, GpuResourceUsageState::WRITE_RENDER_TARGET);
  ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
  GPU_END_PROFILE(ctx);
  RenderCore::ExecuteAndResetCommandList(ctx);

  for (UniquePtr<Test>& testItem : myTests)
    testItem->OnRender();

  ImGui::Render();

  ctx->ResourceBarrier(myRenderOutput->GetBackbuffer(), GpuResourceUsageState::WRITE_RENDER_TARGET, GpuResourceUsageState::READ_PRESENT);
  RenderCore::ExecuteAndFreeCommandList(ctx);
  myRuntime->EndFrame();
}

void Shutdown()
{
  myTests.clear();

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