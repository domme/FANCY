#pragma once
#include "Test.h"
#include "Rendering/ResourceHandle.h"

class Test_SharedQueueResourceUsage : public Test {
public:
  Test_SharedQueueResourceUsage( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                                 Fancy::InputState * anInputState );
  ~Test_SharedQueueResourceUsage() override;
  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  Fancy::GpuBufferViewHandle   myBufferWrite;
  Fancy::GpuBufferViewHandle   myBufferRead;
  Fancy::GpuBufferHandle       myBuffer;

  Fancy::ShaderPipelineHandle myWriteBufferShader;
  Fancy::ShaderPipelineHandle myCopyBufferShader;
};
