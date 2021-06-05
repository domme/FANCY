#pragma once
#include "Test.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_core/ShaderPipeline.h"

class Test_AsyncCompute : public Test
{
public:
  Test_AsyncCompute(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_AsyncCompute() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  enum class Stage
  {
    IDLE = 0,
    WAITING_FOR_READBACK_COPY,
    COPY_DONE,
  };

  Stage myStage = Stage::IDLE;
  uint64 myExpectedBufferValue = 0ull;
  uint64 myBufferCopyFence = 0ull;
  
  Fancy::SharedPtr<Fancy::GpuBufferView> myBufferUAV;
  Fancy::SharedPtr<Fancy::GpuBuffer> myBuffer;
  Fancy::SharedPtr<Fancy::GpuBuffer> myReadbackBuffer;

  Fancy::SharedPtr<Fancy::ShaderPipeline> mySetBufferValueShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myIncrementBufferShader;
};
