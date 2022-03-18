#pragma once
#include "Common/Application.h"
#include "Rendering/GpuBuffer.h"
#include "ProfilerWindow.h"

class Test_Profiler : public Application
{
public:
  Test_Profiler(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Profiler() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  bool myShowProfilerWindow = false;
  Fancy::ProfilerWindow myProfilerWindow;
  Fancy::SharedPtr<Fancy::GpuBuffer> myDummyGpuBuffer1;
  Fancy::SharedPtr<Fancy::GpuBuffer> myDummyGpuBuffer2;
};

