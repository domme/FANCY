#pragma once

/// Enables various sanity-checks and validations
#define FANCY_RENDERER_HEAVY_VALIDATION 1
#define FANCY_RENDERER_DEBUG 1
#define FANCY_RENDERER_DEBUG_MEMORY_ALLOCS FANCY_RENDERER_DEBUG

namespace Fancy {
//---------------------------------------------------------------------------//
  namespace RenderConstants 
  {
    enum 
    {
      kMaxNumRenderTargets = 8u,
      kMaxNumTextureSamplers = 32u,
      kMaxNumReadBuffers = 32u,
      kMaxNumWriteBuffers = 8u,
      kMaxNumReadTextures = 32u,
      kMaxNumWriteTextures = 8u,
      kMaxNumBoundConstantBuffers = 12u,
      kMaxNumGpuProgramResources = 32u,
      kMaxNumConstantBufferElements = 128u,
      kMaxNumGeometriesPerSubModel = 128u,
      kMaxNumRenderContexts = 256u,
      kNumRenderThreads = 1u
    };
  }
//---------------------------------------------------------------------------//
  enum class RenderingApi
  {
    DX12 = 0,
    VULKAN,
  };
//---------------------------------------------------------------------------//
  enum class RenderingTechnique
  {
    FORWARD = 0,
    FORWARD_PLUS,

    NUM
  };
  //---------------------------------------------------------------------------//
  struct RenderingStartupParameters
  {
    RenderingStartupParameters()
      : myRenderingTechnique(RenderingTechnique::FORWARD)
      , myRenderingApi(RenderingApi::DX12)
    { }

    RenderingTechnique myRenderingTechnique;
    RenderingApi myRenderingApi;
  };
//---------------------------------------------------------------------------//
  struct RenderPlatformCaps
  {
    RenderPlatformCaps()
      : myMaxNumVertexAttributes(32)
      , myCbufferPlacementAlignment(0)
    {}

    unsigned int myMaxNumVertexAttributes;
    unsigned int myCbufferPlacementAlignment;
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy