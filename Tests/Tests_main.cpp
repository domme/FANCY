#include <Windows.h>
#include <imgui.h>
#include <imgui_impl_fancy.h>
#include "Rendering/RenderOutput.h"
#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"
#include "Common/Fancy.h"
#include "Common/Window.h"
#include "Debug/Profiler.h"
#include "Common/Input.h"

#include <array>
#include "Common/Application.h"
#include "Test_Profiler.h"
#include "Test_ImGui.h"
#include "Test_GpuMemoryAllocator.h"
#include "Test_Synchronization.h"
#include "Test_AsyncCompute.h"
#include "Test_Mipmapping.h"
#include "Test_ModelViewer.h"
#include "Test_Raytracing.h"
#include "Test_SharedQueueResourceUsage.h"
#include "Test_HazardTracking.h"
#include "Common/Ptr.h"
#include "Common/StringUtil.h"

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 4; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\"; }

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
bool test_mipmapping = false;
bool test_modelviewer = false;
bool test_sharedQueueResources = false;
bool test_hazardTracking = false;
bool test_raytracing = false;

UniquePtr<Test_Profiler> myProfilerTest;
UniquePtr<Test_ImGui> myImGuiTest;
UniquePtr<Test_GpuMemoryAllocator> myGpuMemoryAllocatorTest;
UniquePtr<Test_Synchronization> mySynchronizationTest;
UniquePtr<Test_AsyncCompute> myAsyncComputeTest;
UniquePtr<Test_Mipmapping> myMipmappingTest;
UniquePtr<Test_ModelViewer> myModelViewerTest;
UniquePtr<Test_SharedQueueResourceUsage> mySharedQueueResourceUsageTest;
UniquePtr<Test_HazardTracking> myHazardTrackingTest;
UniquePtr<Test_Raytracing> myRaytracingTest;

eastl::vector<Application*> myTests;

void OnWindowResized(uint aWidth, uint aHeight)
{
  for (Application* testItem : myTests)
  {
    testItem->OnWindowResized(aWidth, aHeight);
  }
}

void Init(HINSTANCE anInstanceHandle, const char** someArguments, uint aNumArguments)
{
  Fancy::WindowParameters windowParams;
  windowParams.myWidth = 1280;
  windowParams.myHeight = 720;
  windowParams.myTitle = "Fancy Engine Tests";

  Fancy::RenderPlatformProperties renderProperties;
  myRuntime = FancyRuntime::Init(anInstanceHandle, someArguments, aNumArguments, windowParams, renderProperties, "../../../../../");

  myRenderOutput = myRuntime->GetRenderOutput();
  myWindow = myRenderOutput->GetWindow();

  std::function<void(uint, uint)> onWindowResized = &OnWindowResized;
  myWindow->myOnResize.Connect(onWindowResized);
  myWindow->myWindowEventHandler.Connect(&myInputState, &InputState::OnWindowEvent);

  myImGuiContext = ImGui::CreateContext();
  ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);
}

template <class T>
void HandleTestDestroyCreate(bool aIsActive, UniquePtr<T>& aTestPtr)
{
  if (!aIsActive)
  {
    myTests.erase_first_unsorted(aTestPtr.get());
    aTestPtr.reset();
  }
  else
  {
    aTestPtr = eastl::make_unique<T>(myRuntime, myWindow, myRenderOutput, &myInputState);
    myTests.push_back(aTestPtr.get());
  }
}

void Update()
{
  myRuntime->BeginFrame();
  ImGuiRendering::NewFrame();
  myRuntime->Update(0.016f);

  ImGui::Checkbox("Log resource barriers", &RenderCore::ourDebugLogResourceBarriers);

  if (ImGui::Checkbox("Test Profiler", &test_profiler))
    HandleTestDestroyCreate(test_profiler, myProfilerTest);
  if (ImGui::Checkbox("Test ImGui", &test_imgui))
    HandleTestDestroyCreate(test_imgui, myImGuiTest);
  if (ImGui::Checkbox("Test Gpu Memory Allocations", &test_gpuMemoryAllocs))
    HandleTestDestroyCreate(test_gpuMemoryAllocs, myGpuMemoryAllocatorTest);
  if (ImGui::Checkbox("Test Async Compute", &test_asyncCompute))
    HandleTestDestroyCreate(test_asyncCompute, myAsyncComputeTest);
  if (ImGui::Checkbox("Test Hazard Tracking", &test_hazardTracking))
    HandleTestDestroyCreate(test_hazardTracking, myHazardTrackingTest);
  if (ImGui::Checkbox("Test Mipmapping", &test_mipmapping))
    HandleTestDestroyCreate(test_mipmapping, myMipmappingTest);
  if (ImGui::Checkbox("Test Model Viewer", &test_modelviewer))
    HandleTestDestroyCreate(test_modelviewer, myModelViewerTest);
  if (ImGui::Checkbox("Test Synchronization", &test_sychronization))
    HandleTestDestroyCreate(test_sychronization, mySynchronizationTest);
  if (ImGui::Checkbox("Test Shared Queue Resources", &test_sharedQueueResources))
    HandleTestDestroyCreate(test_sharedQueueResources, mySharedQueueResourceUsageTest);
  if (RenderCore::GetPlatformCaps().mySupportsRaytracing && ImGui::Checkbox("Test Raytracing", &test_raytracing))
    HandleTestDestroyCreate(test_raytracing, myRaytracingTest);

  ImGui::Separator();

  for (Application* testItem : myTests)
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
  float clearColor[] = { 0.3f, 0.3f, 0.3f, 0.0f };
  ctx->ClearRenderTarget(myRenderOutput->GetBackbufferRtv(), clearColor);
  ctx->ClearDepthStencilTarget(myRenderOutput->GetDepthStencilDsv(), 1.0f, 0u, (uint) DepthStencilClearFlags::CLEAR_ALL);
  GPU_END_PROFILE(ctx);
  RenderCore::ExecuteAndFreeCommandList(ctx);

  for (Application* testItem : myTests)
    testItem->OnRender();

  ImGui::Render();

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
  int numArgs = 0;
  LPWSTR* commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &numArgs);

  eastl::vector<eastl::string> commandLineArgStrings(numArgs);
  eastl::vector<const char*> cStrings(numArgs);
  for (uint i = 0u; i < numArgs; ++i)
  {
    commandLineArgStrings[i] = StringUtil::ToNarrowString(commandLineArgs[i]);
    cStrings[i] = commandLineArgStrings[i].c_str();
  }

  LocalFree(commandLineArgs);

  Init(hInstance, cStrings.data(), cStrings.size());

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