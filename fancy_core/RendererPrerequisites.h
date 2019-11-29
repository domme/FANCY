#pragma once

/// Enables various sanity-checks and validations
#define FANCY_RENDERER_HEAVY_VALIDATION 1
#define FANCY_RENDERER_DEBUG 1
#define FANCY_RENDERER_DEBUG_MEMORY_ALLOCS FANCY_RENDERER_DEBUG

// Track the resource barrier-states per resource. Only works in single-threaded commandList-recording!
#define FANCY_RENDERER_TRACK_RESOURCE_BARRIER_STATES 0
#define FANCY_RENDERER_LOG_RESOURCE_BARRIERS 0

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
      kMaxNumShaderResources = 32u,
      kMaxNumConstantBufferElements = 128u,
      kMaxNumGeometriesPerSubModel = 128u,
      kMaxNumRenderContexts = 256u,
      kNumRenderThreads = 1u
    };
  }
//---------------------------------------------------------------------------//
  enum class RenderPlatformType
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
      , myRenderingApi(RenderPlatformType::DX12)
    { }

    RenderingTechnique myRenderingTechnique;
    RenderPlatformType myRenderingApi;
  };
//---------------------------------------------------------------------------//
  struct RenderPlatformCaps
  {
    RenderPlatformCaps()
      : myMaxNumVertexAttributes(32u)
      , myCbufferPlacementAlignment(0u)
      , myTextureRowAlignment(1u)
      , myTextureSubresourceBufferAlignment(1u)
    {}

    unsigned int myMaxNumVertexAttributes;
    unsigned int myCbufferPlacementAlignment;
    unsigned int myTextureRowAlignment;
    unsigned int myTextureSubresourceBufferAlignment;
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy