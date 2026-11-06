#pragma once

#include "Test.h"
#include "FrameGraphViewer.h"
#include "Rendering/FrameGraph/FrameGraphBuilder.h"
#include "Rendering/ResourceHandle.h"

/// A compute task stores the quadrant colors in a buffer. Four tasks read them to fill the quadrants of a texture (alternating
/// compute UAV writes and render target draws), and a last task blits the texture to the backbuffer.
class Test_FrameGraph : public Test {
public:
  Test_FrameGraph( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                   Fancy::InputState * anInputState );
  ~Test_FrameGraph() override;

  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  void CreateQuadrantTexture();

  enum Consts { kNumQuadrants = 4 };

  Fancy::FrameGraphBuilder myFrameGraph;
  Fancy::FrameGraphViewer myFrameGraphViewer;
  bool myShowFrameGraphViewer = true;
  bool myQuadrantEnabled[ kNumQuadrants ] = { true, true, true, true };

  Fancy::TextureHandle myQuadrantTexture;
  Fancy::GpuBufferHandle myColorBuffer;
  Fancy::ShaderPipelineHandle myStoreColorsShader;
  Fancy::ShaderPipelineHandle myFillQuadrantComputeShader;
  Fancy::ShaderPipelineHandle myFillColorShader;
  Fancy::ShaderPipelineHandle myBlitShader;
  Fancy::DepthStencilStateHandle myDepthTestOff;
};
