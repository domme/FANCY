#pragma once
#include "Test.h"
#include "fancy_core/Ptr.h"

using namespace Fancy;

namespace Fancy
{
  class RaytracingShaderTable;
  struct RaytracingPipelineState;
  class GpuBuffer;
  class RaytracingBVH;
  class Shader;
}

class Test_Raytracing : public Test
{
public:
  Test_Raytracing(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Raytracing() override;

  void OnWindowResized(uint aWidth, uint aHeight) override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

  SharedPtr<GpuBuffer> myRTvertexBuffer;
  SharedPtr<GpuBuffer> myRTindexBuffer;
  SharedPtr<GpuBuffer> myRTtransformBuffer;

  SharedPtr<RaytracingBVH> myBottomLevelBVH;
  SharedPtr<RaytracingBVH> myTopLevelBVH;

  SharedPtr<RaytracingPipelineState> myRtPso;
  SharedPtr<RaytracingShaderTable> myRayGenTable;
  SharedPtr<RaytracingShaderTable> myMissTable;
  SharedPtr<RaytracingShaderTable> myHitTable;
}; 


