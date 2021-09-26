#pragma once
#include "Test.h"
#include "fancy_core/Ptr.h"

using namespace Fancy;

namespace Fancy
{
  class TextureView;
  class Texture;
  class GpuBufferView;
  class RaytracingShaderTable;
  class RaytracingPipelineState;
  class GpuBuffer;
  class RaytracingBVH;
  class Shader;
}

class Test_Raytracing : public Test
{
public:
  Test_Raytracing(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Raytracing() override = default;

  void OnWindowResized(uint aWidth, uint aHeight) override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

  SharedPtr<GpuBuffer> myRTvertexBuffer;
  SharedPtr<GpuBuffer> myRTindexBuffer;
  SharedPtr<GpuBuffer> myRTtransformBuffer;

  SharedPtr<RaytracingBVH> myBottomLevelBVH;
  SharedPtr<RaytracingBVH> myTopLevelBVH;

  SharedPtr<RaytracingPipelineState> myRtPso;
  SharedPtr<RaytracingShaderTable> myRaygenShaderTable;
  SharedPtr<RaytracingShaderTable> myMissShaderTable;
  SharedPtr<RaytracingShaderTable> myHitShaderTable;
}; 


