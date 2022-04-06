#pragma once
#include "Test.h"
#include "Rendering/GpuBuffer.h"

class Test_Synchronization : public Test
{
public:
  Test_Synchronization(Fancy::AssetManager* anAssetManager, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
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
