#pragma once
#include "Test.h"
#include "GpuBuffer.h"
#include "Texture.h"

#include "EASTL/vector.h"

class Test_GpuMemoryAllocator : public Test
{
public:
  Test_GpuMemoryAllocator(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow, Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState);
  ~Test_GpuMemoryAllocator() override;
  void OnUpdate(bool aDrawProperties) override;
  void OnRender() override;

private:
  void RenderMemoryAllocatorLayouts();
  bool myAllocatorTypeVisible[(uint)Fancy::GpuMemoryType::NUM][(uint)Fancy::CpuMemoryAccessType::NUM];

  int myBufferToAllocSizeMb;
  float myScale;
  eastl::vector<Fancy::SharedPtr<Fancy::GpuBuffer>> myBuffers;
  eastl::vector<Fancy::SharedPtr<Fancy::Texture>> myTextures;
};

