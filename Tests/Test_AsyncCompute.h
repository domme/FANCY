#pragma once
#include "Test.h"
#include "Rendering/ResourceHandle.h"

class Test_AsyncCompute : public Test {
public:
  Test_AsyncCompute( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                     Fancy::InputState * anInputState );
  ~Test_AsyncCompute() override;
  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  enum class Stage {
    IDLE = 0,
    WAITING_FOR_READBACK_COPY,
    COPY_DONE,
  };

  Stage  myStage = Stage::IDLE;
  uint64 myExpectedBufferValue = 0ull;
  uint64 myBufferCopyFence = 0ull;

  Fancy::GpuBufferViewHandle myBufferUAV;
  Fancy::GpuBufferHandle     myBuffer;
  Fancy::GpuBufferHandle     myReadbackBuffer;

  Fancy::ShaderPipelineHandle mySetBufferValueShader;
  Fancy::ShaderPipelineHandle myIncrementBufferShader;
};
