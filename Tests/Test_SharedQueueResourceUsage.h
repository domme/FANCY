#pragma once
#include "Test.h"
#include "GpuBuffer.h"
#include "ShaderPipeline.h"

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

