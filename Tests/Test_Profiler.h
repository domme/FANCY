#pragma once
#include "Test.h"
#include "fancy_core/GpuBuffer.h"
#include "app_framework/ProfilerWindow.h"

class Test_Profiler : public Test
{
public:
  Test_Profiler(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Profiler() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  bool myShowProfilerWindow = false;
  ProfilerWindow myProfilerWindow;
  Fancy::SharedPtr<Fancy::GpuBuffer> myDummyGpuBuffer1;
  Fancy::SharedPtr<Fancy::GpuBuffer> myDummyGpuBuffer2;
};

