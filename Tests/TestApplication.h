#pragma once
#include <EASTL/vector.h>

#include "Common/Application.h"
#include "Common/Ptr.h"

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
#include "Rendering/RenderOutput.h"

namespace Fancy
{
  class Window;
  class RenderOutput;
  struct RenderPlatformProperties;
  struct WindowParameters;
}

  class TestApplication : public Fancy::Application
  {
  public:
    TestApplication(HINSTANCE anInstanceHandle,
      const char** someArguments,
      uint aNumArguments,
      const char* aName,
      const Fancy::RenderPlatformProperties& someRenderProperties,
      const Fancy::WindowParameters& someWindowParams);

    ~TestApplication() override;
    void OnWindowResized(uint aWidth, uint aHeight) override;
    void BeginFrame() override;
    void Update() override;
    void Render() override;
    void EndFrame() override;

  private:
    template <class T>
    void HandleTestDestroyCreate(bool aIsActive, Fancy::UniquePtr<T>& aTestPtr)
    {
      if (!aIsActive)
      {
        myTests.erase_first_unsorted(aTestPtr.get());
        aTestPtr.reset();
      }
      else
      {
        aTestPtr = eastl::make_unique<T>(myAssetManager.get(), myRenderOutput->GetWindow(), myRenderOutput.get(), &myInputState);
        myTests.push_back(aTestPtr.get());
      }
    }

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

    Fancy::UniquePtr<Test_Profiler> myProfilerTest;
    Fancy::UniquePtr<Test_ImGui> myImGuiTest;
    Fancy::UniquePtr<Test_GpuMemoryAllocator> myGpuMemoryAllocatorTest;
    Fancy::UniquePtr<Test_Synchronization> mySynchronizationTest;
    Fancy::UniquePtr<Test_AsyncCompute> myAsyncComputeTest;
    Fancy::UniquePtr<Test_Mipmapping> myMipmappingTest;
    Fancy::UniquePtr<Test_ModelViewer> myModelViewerTest;
    Fancy::UniquePtr<Test_SharedQueueResourceUsage> mySharedQueueResourceUsageTest;
    Fancy::UniquePtr<Test_HazardTracking> myHazardTrackingTest;
    Fancy::UniquePtr<Test_Raytracing> myRaytracingTest;
    
    eastl::vector<Test*> myTests;

    ImGuiContext* myImGuiContext = nullptr;
  };



