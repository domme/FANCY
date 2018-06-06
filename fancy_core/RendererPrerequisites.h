#pragma once

#include "FancyCorePrerequisites.h"
#include "DataFormat.h"

/// Enables various sanity-checks and validations
#define FANCY_RENDERER_HEAVY_VALIDATION 1

namespace Fancy {
//---------------------------------------------------------------------------//
    namespace MultiBuffering 
    {
      enum { kGpuMultiBufferingCount = 2u };  
      uint getCurrentBufferIndex();
    }
//---------------------------------------------------------------------------//
    namespace Constants 
    {
      enum 
      {
        kMaxNumRenderTargets = 7u, // (8 color-rts -1 for depth-stencil target)
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
    class Texture;
    class TextureSampler;
    class GpuBuffer;
    class GpuProgram;
    class GpuProgramPipeline;
    class GpuProgramCompiler;
    class GeometryData;
  //---------------------------------------------------------------------------//
    enum class TopologyType
    {
      TRIANGLE_LIST,
      LINES,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class CompFunc 
    {
      NEVER = 0,
      LESS,
      EQUAL,
      LEQUAL,
      GREATER,
      NOTEQUAL,
      GEQUAL,
      ALWAYS,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class StencilOp 
    {
      KEEP = 0,
      ZERO,
      REPLACE,
      INCREMENT_CLAMP,
      DECREMENT_CLAMP,
      INVERT,
      INCEMENT_WRAP,
      DECREMENT_WRAP,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class FillMode 
    {
      WIREFRAME = 0,
      SOLID,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class CullMode 
    { 
      NONE = 0,
      FRONT,
      BACK,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class WindingOrder 
    {
      CCW = 0,
      CW,

      NUM
    };
  //-----------------------------------------------------------------------//    
    enum class BlendInput 
    {
      ZERO = 0,
      ONE,
      SRC_COLOR,
      INV_SRC_COLOR,
      SRC_ALPHA,
      INV_SRC_ALPHA,
      DEST_ALPHA,
      INV_DEST_ALPHA,
      DEST_COLOR,
      INV_DEST_COLOR,
      SRC_ALPHA_CLAMPED,
      BLEND_FACTOR,  // DX11-only
      INV_BLEND_FACTOR,
      SRC1_COLOR,
      INV_SRC1_COLOR,
      SRC1_ALPHA,
      INV_SRC1_ALPHA,

      // OpenGL-only:
      CONSTANT_COLOR,
      INV_CONSTANT_COLOR,
      CONSTANT_ALPHA,
      INV_CONSTANT_ALPHA,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class BlendOp 
    {
      ADD = 0,
      SUBTRACT,
      REV_SUBTRACT,
      MIN,
      MAX,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class ShaderStage 
    {
      VERTEX        = 0,
      FRAGMENT,      
      GEOMETRY,      
      TESS_HULL,     
      TESS_DOMAIN,
      COMPUTE,                  
      
      NUM,
      NUM_NO_COMPUTE = NUM - 1,
      NONE
    };
  //---------------------------------------------------------------------------//
    enum class ShaderStageFlag 
    {
      VERTEX        = 0x01,
      FRAGMENT      = 0x02,      
      GEOMETRY      = 0x04,      
      TESS_HULL     = 0x08,     
      TESS_DOMAIN   = 0x10,   
      COMPUTE       = 0x20,                  

      ALL           = 0xFF
    };
  //-----------------------------------------------------------------------//
    enum class FaceType 
    {
      FRONT = 0,
      BACK,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class GpuResourceState 
    {
      RESOURCE_STATE_COMMON = 0,
      RESOURCE_STATE_GENERIC_READ,
      RESOURCE_STATE_DEPTH_WRITE,
      RESOURCE_STATE_DEPTH_READ,
      RESOURCE_STATE_PRESENT,
      RESOURCE_STATE_COPY_DEST,
      RESOURCE_STATE_COPY_SRC,
      RESOURCE_STATE_RENDER_TARGET,

      NONE
    };
  //---------------------------------------------------------------------------//
    enum class GpuMemoryAccessType
    {
      NO_CPU_ACCESS = 0,
      CPU_WRITE,
      CPU_READ,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class GpuResoruceLockOption 
    {
      READ = 0,
      WRITE,
      READ_WRITE,
            
      NUM
    };
//---------------------------------------------------------------------------//
    struct TextureParams 
    {
      TextureParams() : path(""), u16Width(0u), u16Height(0u), myDepthOrArraySize(0u),
        eFormat(DataFormat::NONE), myAccessType(0u), bIsDepthStencil(false), myIsExternalTexture(true), myInternalRefIndex(~0u), myNumMipLevels(0u),
        myIsShaderWritable(false), myIsRenderTarget(false) {}

      uint16 u16Width;
      uint16 u16Height;
      uint16 myDepthOrArraySize;
      DataFormat eFormat;
      uint myAccessType;
      bool myIsShaderWritable;
      bool myIsRenderTarget;
      bool bIsDepthStencil;
      String path;
      bool myIsExternalTexture;
      uint myInternalRefIndex;

      uint8 myNumMipLevels;
    };
//---------------------------------------------------------------------------//
    struct TextureSubLayout
    {
      DataFormat myFormat;
      uint myWidth;
      uint myHeight;
      uint myDepth;
      uint64 myAlignedRowSize;
      uint64 myRowSize;
      uint myNumRows;
    };
 //---------------------------------------------------------------------------//
    struct TextureSubLocation
    {
      TextureSubLocation() : myArrayIndex(0u), myMipLevel(0u), myPlaneIndex(0u) {}
      uint myArrayIndex;
      uint myMipLevel;
      uint myPlaneIndex;
    };
  //---------------------------------------------------------------------------//
    struct TextureRegion
    {
      glm::uvec3 myTexelPos;
      glm::uvec3 myTexelSize;
    };
 //---------------------------------------------------------------------------//
    struct TextureSubData
    {
      TextureSubData()
        : myData(nullptr), myRowSizeBytes(0u), myPixelSizeBytes(0u), mySliceSizeBytes(0u), myTotalSizeBytes(0u)
      {}
    
      TextureSubData(const TextureParams& someParams);
      
      uint8* myData;
      uint64 myPixelSizeBytes;
      uint64 myRowSizeBytes;
      uint64 mySliceSizeBytes;
      uint64 myTotalSizeBytes;
    };
//---------------------------------------------------------------------------//
    enum class GpuBufferUsage 
    {
      STAGING_UPLOAD = 0,
      STAGING_READBACK,
      CONSTANT_BUFFER,
      VERTEX_BUFFER,
      INDEX_BUFFER,
      SHADER_BUFFER
    };
 //---------------------------------------------------------------------------//
    struct GpuBufferCreationParams 
    {
      GpuBufferCreationParams() : uNumElements(0u), myInternalRefIndex(~0u), myCreateDerivedViews(true), 
        uElementSizeBytes(0u), myUsageFlags(0u), myAccessType(0u), myIsShaderWritable(false) {}

      uint uNumElements;
      uint uElementSizeBytes;
      uint myAccessType;
      uint myInternalRefIndex;
      uint myUsageFlags;
      bool myCreateDerivedViews;
      bool myIsShaderWritable;
    };
 //---------------------------------------------------------------------------//
    enum class VertexSemantics 
    {
      NONE = 0,

      POSITION,
      NORMAL,
      TANGENT,
      BITANGENT,
      COLOR,
      TEXCOORD,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class SamplerFilterMode 
    {
      NEAREST = 0,
      BILINEAR,
      TRILINEAR,
      ANISOTROPIC,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class SamplerAddressMode 
    {
      WRAP = 0,
      CLAMP_EDGE,
      MIRROR_CLAMP_EDGE,
      CLAMP_BORDER,
      REPEAT,
      MIRROR_REPEAT,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class DepthStencilClearFlags
    {
      CLEAR_DEPTH = (1 << 0),
      CLEAR_STENCIL = (1 << 1),
      CLEAR_ALL = 0xFF
    };
//---------------------------------------------------------------------------//
    enum class GpuDescriptorTypeFlags
    {
      READ_BUFFER = (1 << 0),
      READ_TEXTURE = (1 << 1),
      WRITE_BUFFER = (1 << 2),
      WRITE_TEXTURE = (1 << 3),
      CONSTANT_BUFFER = (1 << 4),
      BUFFER_TEXTURE_CONSTANT_BUFFER = READ_BUFFER | READ_TEXTURE | WRITE_BUFFER | WRITE_TEXTURE | CONSTANT_BUFFER,
      SAMPLER = (1 << 5),
      RENDER_TARGET = (1 << 6),
      VERTEX_BUFFER = (1 << 7),
      INDEX_BUFFER = (1 << 8),
      DEPTH_STENCIL_TEXTURE = (1 << 9)
    };
//---------------------------------------------------------------------------//
    enum class GpuMemoryType
    {
      BUFFER = 0,
      TEXTURE,
      RENDERTARGET,

      NUM
    };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy