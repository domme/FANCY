#pragma once

/// Enables various sanity-checks and validations
#define FANCY_RENDERER_USE_VALIDATION 1
#define FANCY_HEAVY_DEBUG 1
#define FANCY_RENDERER_DEBUG_MEMORY_ALLOCS FANCY_HEAVY_DEBUG

#define FANCY_RENDERER_LOG_RESOURCE_BARRIERS 1

#define FANCY_RENDERER_SUPPORT_MESH_SHADERS 0
#define FANCY_RENDERER_SUPPORT_CONDITIONAL_RENDERING 0
#define FANCY_RENDERER_SUPPORT_VRS 0
#define FANCY_RENDERER_SUPPORT_FRAGMENT_DENSITY_MAP 0

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
      kNumRenderThreads = 1u,
      kDefaultNumAttributes = 16u,
    };
  }
//---------------------------------------------------------------------------//
  enum class RenderPlatformType
  {
    DX12 = 0,
    VULKAN,
  };
//---------------------------------------------------------------------------//
  struct RenderPlatformCaps
  {
    unsigned int myMaxNumVertexAttributes = 32u;
    unsigned int myCbufferPlacementAlignment = 1u;
    unsigned int myTextureRowAlignment = 1u;
    unsigned int myTextureSubresourceBufferAlignment = 10;
    unsigned int myMaxTextureAnisotropy = 16u;
    unsigned int myRaytracingShaderIdentifierSizeBytes = 0u;
    unsigned int myRaytracingShaderTableAddressAlignment = 0u;
    unsigned int myRaytracingShaderRecordAlignment = 0u;
    unsigned int myRaytracingMaxShaderRecordSize = 0u;
    unsigned int myRaytracingMaxRecursionDepth = 0u;
    bool myHasAsyncCompute = false;
    bool myHasAsyncCopy = false;
    bool mySupportsRaytracing = false;
  };
//---------------------------------------------------------------------------//
  struct RenderPlatformProperties
  {
    unsigned int myNumGlobalTextures1D = 64u;
    unsigned int myNumGlobalTextures2D = 2048u;
    unsigned int myNumGlobalTextures3D = 1024u;
    unsigned int myNumGlobalTexturesCube = 128u;
    unsigned int myNumGlobalBuffers = 2048u;
    unsigned int myNumGlobalSamplers = 1024u;
    unsigned int myNumLocalBuffers = 8u;
    unsigned int myNumLocalCBuffers = 8u;
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy