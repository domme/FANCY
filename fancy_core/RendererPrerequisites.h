#pragma once

#include "FancyCorePrerequisites.h"
#include "DataFormat.h"

/// Enables various sanity-checks and validations
#define FANCY_RENDERER_HEAVY_VALIDATION 1
#define FANCY_RENDERER_DEBUG 1

namespace Fancy {
//---------------------------------------------------------------------------//
    namespace Constants 
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
    enum class GpuResourceTransition
    {
      TO_READ_GRAPHICS_COMPUTE,								// make a resource written by default/compute contexts readable by any default/compute context
      TO_READ_WRITE_DMA,								// make a resource written by any context read/writable by DMA context
      TO_COMMON,
      TO_PRESENT,
      TO_RENDERTARGET,
      TO_COPY_DEST,
      TO_COPY_SRC,
      TO_SHADER_READ,
      TO_SHADER_WRITE,
    };
  //---------------------------------------------------------------------------//
    enum class CpuMemoryAccessType
    {
      NO_CPU_ACCESS = 0,
      CPU_WRITE,
      CPU_READ,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class GpuResourceMapMode 
    {
      READ = 0,
      READ_UNSYNCHRONIZED,
      WRITE_UNSYNCHRONIZED,
      WRITE,
            
      NUM
    };
//---------------------------------------------------------------------------//
    enum class GpuResourceDimension
    {
      UNKONWN = 0,
      BUFFER,
      TEXTURE_1D,
      TEXTURE_2D,
      TEXTURE_3D,
      TEXTURE_CUBE,
      TEXTURE_1D_ARRAY,
      TEXTURE_2D_ARRAY,
      TEXTURE_CUBE_ARRAY
    };
//---------------------------------------------------------------------------//
    struct TextureProperties 
    {
      TextureProperties() 
        : myDimension(GpuResourceDimension::UNKONWN)
        , path("")
        , myWidth(0u)
        , myHeight(0u)
        , myDepthOrArraySize(1u)
        , eFormat(DataFormat::NONE)
        , myAccessType(CpuMemoryAccessType::NO_CPU_ACCESS)
        , myNumMipLevels(UINT_MAX)
        , bIsDepthStencil(false)
        , myIsShaderWritable(false)
        , myIsRenderTarget(false) 
        , myPreferTypedFormat(false)
      {}

      bool IsArray() const { return myDimension == GpuResourceDimension::TEXTURE_1D_ARRAY || myDimension == GpuResourceDimension::TEXTURE_2D_ARRAY || myDimension == GpuResourceDimension::TEXTURE_CUBE_ARRAY; }
      uint GetArraySize() const { return IsArray() ? myDepthOrArraySize : 1u; }
      uint GetDepthSize() const { return IsArray() ? 1u : myDepthOrArraySize; }

      GpuResourceDimension myDimension;
      String path;
      uint myWidth;
      uint myHeight;
      uint myDepthOrArraySize;
      DataFormat eFormat;
      CpuMemoryAccessType myAccessType;
      uint myNumMipLevels;
      bool bIsDepthStencil;
      bool myIsShaderWritable;
      bool myIsRenderTarget;
      bool myPreferTypedFormat;
    };
//---------------------------------------------------------------------------//
    struct TextureSubLayout
    {
      TextureSubLayout() 
        : myWidth(0u)
        , myHeight(0u)
        , myDepth(0u)
        , myAlignedRowSize(0u)
        , myRowSize(0u)
        , myNumRows(0u)
      {}

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
      TextureSubLocation(uint aMipLevel = 0u, uint anArrayIndex = 0u, uint aPlaneIndex = 0u) 
      : myMipLevel(aMipLevel), myArrayIndex(anArrayIndex), myPlaneIndex(aPlaneIndex) {}
      uint myMipLevel;
      uint myArrayIndex;
      uint myPlaneIndex;
    };
  //---------------------------------------------------------------------------//
    struct TextureRegion
    {
      TextureRegion()
        : myTexelPos(glm::uvec3(0))
        , myTexelSize(glm::uvec3(UINT_MAX))
      {}

      static TextureRegion ourMaxRegion;

      glm::uvec3 myTexelPos;
      glm::uvec3 myTexelSize;
    };
 //---------------------------------------------------------------------------//
    struct TextureSubData
    {
      TextureSubData()
        : myData(nullptr), myRowSizeBytes(0u), myPixelSizeBytes(0u), mySliceSizeBytes(0u), myTotalSizeBytes(0u)
      {}
    
      TextureSubData(const TextureProperties& someProperties);
      
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
    struct GpuBufferProperties 
    {
      GpuBufferProperties() 
        : myNumElements(0u)
        , myElementSizeBytes(0u)
        , myCpuAccess(CpuMemoryAccessType::NO_CPU_ACCESS)
        , myUsage(GpuBufferUsage::CONSTANT_BUFFER)
        , myIsShaderWritable(false) 
      { }

      uint64 myNumElements;
      uint64 myElementSizeBytes;
      CpuMemoryAccessType myCpuAccess;
      GpuBufferUsage myUsage;
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