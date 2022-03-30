#include "TestApplication.h"

#include "imgui.h"
#include "imgui_impl_fancy.h"
#include "Test.h"
#include "Common/Fancy.h"
#include "Common/Ptr.h"
#include "Rendering/RenderCore.h"

using namespace Fancy;

  TestApplication::TestApplication(HINSTANCE anInstanceHandle, const char** someArguments, uint aNumArguments,
    const char* aName, const RenderPlatformProperties& someRenderProperties, const WindowParameters& someWindowParams)
      : Application(anInstanceHandle, someArguments, aNumArguments, aName, "../../../../../", someRenderProperties, someWindowParams)
      , myImGuiContext(ImGui::CreateContext())
  {
    ImGuiRendering::Init(myRuntime->GetRenderOutput(), myRuntime);
  }

  TestApplication::~TestApplication()
  {
    myTests.clear();
    ImGuiRendering::Shutdown();
    ImGui::DestroyContext(myImGuiContext);
    myImGuiContext = nullptr;
  }

  void TestApplication::OnWindowResized(uint aWidth, uint aHeight)
  {
    Application::OnWindowResized(aWidth, aHeight);

    for (Test* testItem : myTests)
    {
      testItem->OnWindowResized(aWidth, aHeight);
    }
  }

void TestApplication::BeginFrame()
{
  Application::BeginFrame();
  ImGuiRendering::NewFrame();
}

void TestApplication::Update()
  {
    Application::Update();

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

    for (Test* testItem : myTests)
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

  void TestApplication::Render()
  {
    Application::Render();

    for (Test* testItem : myTests)
      testItem->OnRender();

    ImGui::Render();
  }

  void TestApplication::EndFrame()
  {
    Application::EndFrame();
  }
