#pragma once
#include "Rendering/ResourceHandle.h"
#include "ProfilerWindow.h"
#include "Test.h"

class Test_Profiler : public Test {
public:
  Test_Profiler( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState );
  ~Test_Profiler() override;
  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  bool                         myShowProfilerWindow = false;
  Fancy::ProfilerWindow        myProfilerWindow;
  Fancy::GpuBufferHandle       myDummyGpuBuffer1;
  Fancy::GpuBufferHandle       myDummyGpuBuffer2;
};
