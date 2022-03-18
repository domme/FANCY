#pragma once
#include "Common/Application.h"
#include "Rendering/GpuBuffer.h"

class Test_Synchronization : public Application
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
