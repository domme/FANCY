#pragma once
#include "Test.h"
#include "fancy_core/Ptr.h"

namespace Fancy
{
  class GpuBuffer;
  class RaytracingBVH;
}

class Test_Raytracing : public Test
{
public:
  Test_Raytracing(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_Raytracing() override;

  void OnWindowResized(uint aWidth, uint aHeight) override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

  Fancy::SharedPtr<Fancy::GpuBuffer> myRTvertexBuffer;
  Fancy::SharedPtr<Fancy::GpuBuffer> myRTindexBuffer;
  Fancy::SharedPtr<Fancy::GpuBuffer> myRTtransformBuffer;

  Fancy::SharedPtr<Fancy::RaytracingBVH> myBottomLevelBVH;
  Fancy::SharedPtr<Fancy::RaytracingBVH> myTopLevelBVH;
}; 

