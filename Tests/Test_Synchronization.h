#pragma once
#include "Test.h"
#include "fancy_core/GpuBuffer.h"

#if BINDLESS_ENABLE_ALL_TESTS

class Test_Synchronization : public Test
{
public:
  Test_Synchronization(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Synchronization() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  bool myWaitForResults = false;

  enum class Stage
  {
    IDLE = 0,
    WAITING_FOR_COPY,
    COPY_DONE
  };

  Stage myStage = Stage::IDLE;
  uint64 myExpectedBufferValue = 0ull;
  uint64 myBufferCopyFence = 0ull;
  Fancy::SharedPtr<Fancy::GpuBuffer> myUploadBuffer;
  Fancy::SharedPtr<Fancy::GpuBuffer> myReadbackBuffer;
};

#endif