#pragma once

#include "Test.h"
#include "FrameGraphViewer.h"
#include "Rendering/FrameGraph/FrameGraphBuilder.h"
#include "Rendering/ResourceHandle.h"

/// A compute task stores the quadrant colors in a buffer. Four tasks read them to fill the quadrants of a texture (alternating
/// compute UAV writes and render target draws), and a last task blits the texture to the backbuffer.
/// Also tests subresources: a mip chain downsampled by compute tasks (one task per mip), and a depth-stencil texture whose stencil plane
/// is drawn by a raster task and copied by a compute task. The blit task shows all of them.
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

  Fancy::TextureHandle myMipChainTexture;
  Fancy::TextureHandle myDepthStencilTexture;
  Fancy::TextureHandle myStencilCopyTexture;
  Fancy::ShaderPipelineHandle myDownsampleShader;
  Fancy::ShaderPipelineHandle myCopyStencilShader;
  Fancy::ShaderPipelineHandle myFillStencilShader;
  Fancy::DepthStencilStateHandle myStencilStates[ 3 ];
};
