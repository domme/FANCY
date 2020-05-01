#pragma once
#include "Test.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_core/ShaderPipeline.h"
#include "fancy_core/Texture.h"

class Test_HazardTracking : public Test
{
public:
  Test_HazardTracking(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
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
