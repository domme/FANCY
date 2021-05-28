#pragma once
#include "Test.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_core/ShaderPipeline.h"

#if BINDLESS_ENABLE_ALL_TESTS

class Test_SharedQueueResourceUsage : public Test
{
public:
  Test_SharedQueueResourceUsage(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_SharedQueueResourceUsage() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  Fancy::SharedPtr<Fancy::GpuBufferView> myBufferWrite;
  Fancy::SharedPtr<Fancy::GpuBufferView> myBufferRead;
  Fancy::SharedPtr<Fancy::GpuBuffer> myBuffer;

  Fancy::SharedPtr<Fancy::ShaderPipeline> myWriteBufferShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myCopyBufferShader;
};

#endif