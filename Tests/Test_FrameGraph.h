#pragma once

#include "Test.h"
#include "FrameGraphViewer.h"
#include "Rendering/FrameGraph/FrameGraphBuilder.h"
#include "Rendering/ResourceHandle.h"

/// A compute task stores the quadrant colors in a buffer. Four tasks read them to fill the quadrants of a texture (alternating
/// compute UAV writes and render target draws), and a last task blits the texture to the backbuffer.
/// Also tests subresources: a mip chain downsampled by compute tasks (one task per mip), and a depth-stencil texture whose stencil plane
/// is drawn by a raster task and copied by a compute task. The blit task shows all of them.
/// Tests the versions of the graph: the tasks chain through the versions their writes return, and the stencil copy reads the depth-stencil texture
/// from before a task overwrites it, so the graph has to run the copy first. RunBuilderChecks() asserts the dependencies of small dummy graphs.
/// Every other frame skips the setup and only rebuilds the graph from the declaration of the last setup.
class Test_FrameGraph : public Test {
public:
  Test_FrameGraph( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                   Fancy::InputState * anInputState );
  ~Test_FrameGraph() override;

  void OnUpdate( bool aDrawProperties ) override;
  void OnRender() override;

private:
  void CreateQuadrantTexture();
  void RunBuilderChecks();
  void SetupFrameGraph();

  enum Consts { kNumQuadrants = 4 };

  Fancy::FrameGraphBuilder myFrameGraph;
  Fancy::FrameGraphViewer myFrameGraphViewer;
  uint myFrameIndex = 0u;
  bool myShowFrameGraphViewer = true;
  bool mySkipSetupEveryOtherFrame = true;
  bool myNeedsSetup = true;  // The last setup references a resource that was recreated since
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
