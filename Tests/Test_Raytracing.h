#pragma once
#include "Test.h"
#include "Common/Ptr.h"

using namespace Fancy;

namespace Fancy
{
  class TextureView;
  class Texture;
  class GpuBufferView;
  class RtShaderBindingTable;
  class RtPipelineState;
  class GpuBuffer;
  class RtAccelerationStructure;
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

  SharedPtr<RtAccelerationStructure> myBLAS;
  SharedPtr<RtAccelerationStructure> myTLAS;

  SharedPtr<RtPipelineState> myRtPso;
  SharedPtr<RtShaderBindingTable> mySBT;
}; 


