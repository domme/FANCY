#pragma once
#include "Test.h"
#include "GpuMemoryViewer.h"
#include "Rendering/ResourceHandle.h"

#include "EASTL/vector.h"

class Test_GpuMemoryAllocator : public Test {
public:
  Test_GpuMemoryAllocator( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow,
                           Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState );
  ~Test_GpuMemoryAllocator() override;
  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  int                                      myBufferToAllocSizeMb;
  Fancy::GpuMemoryViewer                   myGpuMemoryViewer;
  eastl::vector< Fancy::GpuBufferHandle > myBuffers;
  eastl::vector< Fancy::TextureHandle >   myTextures;
};
