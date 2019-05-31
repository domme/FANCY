#pragma once
#include "Test.h"
#include "fancy_core/GpuBuffer.h"

class Test_Synchronization : public Test
{
public:
  Test_Synchronization(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Synchronization() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  bool myTestCpuGpu = false;
  bool myTestAsnycGpuGpu = false;
  Fancy::SharedPtr<Fancy::GpuBuffer> myBufferCpuSync;
  Fancy::SharedPtr<Fancy::GpuBuffer> myBufferAsyncGpu;
};