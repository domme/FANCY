#pragma once
#include "Test.h"
#include "Rendering/ResourceHandle.h"

class Test_HazardTracking : public Test {
public:
  Test_HazardTracking( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow,
                       Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState );
  ~Test_HazardTracking() override;
  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  Fancy::TextureHandle     myTex;
  Fancy::TextureViewHandle myTexMipWrite[ 3 ];
  Fancy::TextureViewHandle myTexMipRead[ 3 ];

  Fancy::GpuBufferHandle     myBuffer;
  Fancy::GpuBufferViewHandle myBufferWrite[ 3 ];
  Fancy::GpuBufferViewHandle myBufferRead[ 3 ];

  Fancy::ShaderPipelineHandle myBufferToMipShader;
  Fancy::ShaderPipelineHandle myMipToBufferShader;
};
