#pragma once
#include "Test.h"
#include "Rendering/GpuBuffer.h"
#include "Rendering/ShaderPipeline.h"
#include "Rendering/Texture.h"

class Test_HazardTracking : public Test
{
public:
  Test_HazardTracking(Fancy::AssetManager* anAssetManager, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_HazardTracking() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  Fancy::SharedPtr<Fancy::Texture> myTex;
  Fancy::SharedPtr<Fancy::TextureView> myTexMipWrite[3];
  Fancy::SharedPtr<Fancy::TextureView> myTexMipRead[3];

  Fancy::SharedPtr<Fancy::GpuBuffer> myBuffer;
  Fancy::SharedPtr<Fancy::GpuBufferView> myBufferWrite[3];
  Fancy::SharedPtr<Fancy::GpuBufferView> myBufferRead[3];
  
  Fancy::SharedPtr<Fancy::ShaderPipeline> myBufferToMipShader;
  Fancy::SharedPtr<Fancy::ShaderPipeline> myMipToBufferShader;
};

