#pragma once

#include "FancyCorePrerequisites.h"
#include "DataFormat.h"
#include "RenderingStartupParameters.h"

//---------------------------------------------------------------------------//
namespace Fancy {
//---------------------------------------------------------------------------//
  namespace Geometry {
    class GeometryData;
  }
//---------------------------------------------------------------------------//
  namespace Rendering {
//---------------------------------------------------------------------------//
    namespace MultiBuffering {
      enum { kGpuMultiBufferingCount = 2u };
      
      uint getCurrentBufferIndex();
    }
//---------------------------------------------------------------------------//
    namespace Constants {
      enum {
        kMaxNumRenderTargets = 7u, // (-1 for depth-stencil target)
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
  // Forward-declarations of common rendering classes
    class Material;
    class Texture;
    class TextureSampler;
    class GpuBuffer;
    class GpuProgram;
    class GpuProgramPipeline;
    class GpuProgramCompiler;
  //---------------------------------------------------------------------------//
    enum class CompFunc {
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
    enum class StencilOp {
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
    enum class FillMode {
      WIREFRAME = 0,
      SOLID,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class CullMode { 
      NONE = 0,
      FRONT,
      BACK,

      NUM
    };
  //-----------------------------------------------------------------------//
    enum class WindingOrder {
      CCW = 0,
      CW,

      NUM
    };
  //-----------------------------------------------------------------------//    
    enum class BlendInput {
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
    enum class BlendOp {
      ADD = 0,
      SUBTRACT,
      REV_SUBTRACT,
      MIN,
      MAX,

      NUM
    };
  //---------------------------------------------------------------------------//
    enum class ShaderStage {
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
    enum class ShaderStageFlag {
      VERTEX        = 0x01,
      FRAGMENT      = 0x02,      
      GEOMETRY      = 0x04,      
      TESS_HULL     = 0x08,     
      TESS_DOMAIN   = 0x10,   
      COMPUTE       = 0x20,                  

      ALL           = 0xFF
    };
  //-----------------------------------------------------------------------//
    enum class FaceType {
      FRONT = 0,
      BACK,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class GpuResourceState {
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
    enum class GpuResourceAccessFlags {
      /// No special access flags
      NONE                = 0x00000000,
      /// Allow CPU read-access only
      READ                = 0x00000001,
      /// Allow CPU write-access only
      WRITE               = 0x00000002,
      /// CPU will change data frequently
      DYNAMIC              = 0x00000008,
      /// CPU-access may remain valid even during GPU-access
      PERSISTENT_LOCKABLE = 0x00000010, //GL4: GL_MAP_PERSISTENT_BIT
      /// Changes from CPU/GPU are immediately visible to GPU/CPU
      COHERENT            = 0x00000020,
      /// Try to use CPU-RAM as backing storage
      PREFER_CPU_STORAGE  = 0x00000040
    };
  //---------------------------------------------------------------------------//
    // Note: Could make bitfield-flags instead, but this would suggest that each field
    // can be combined - even in DX11. Instead TODO: Reduce/expand this set of options during experiments
    enum class GpuResoruceLockOption {
      READ = 0,
      WRITE,
      READ_WRITE,
      WRITE_DISCARD,
      READ_WRITE_DISCARD,
      READ_UNSYNCHRONIZED,  // GL4-only?
      WRITE_UNSYNCHRONIZED,
      READ_WRITE_UNSYNCHRONIZED,
      READ_PERSISTENT,      // GL4-only?
      WRITE_PERSISTENT,
      READ_WRITE_PERSISTENT,
      READ_PERSISTENT_COHERENT,
      WRITE_PERSISTENT_COHERENT,
      READ_WRITE_PERSISTENT_COHERENT,
      
      NUM
    };
  //---------------------------------------------------------------------------//
    struct TextureParams {
      TextureParams() : path(""), u16Width(0u), u16Height(0u), u16Depth(0u),
        eFormat(DataFormat::NONE), uAccessFlags(0u), bIsDepthStencil(false), myIsExternalTexture(true), myInternalRefIndex(~0u), u8NumMipLevels(0u), 
        myIsShaderWritable(false), myIsRenderTarget(false) {}

      uint16 u16Width;
      uint16 u16Height;
      uint16 u16Depth;
      DataFormat eFormat;
      uint uAccessFlags;
      bool myIsShaderWritable;
      bool myIsRenderTarget;
      bool bIsDepthStencil;
      String path;
      bool myIsExternalTexture;
      uint myInternalRefIndex;

      uint8 u8NumMipLevels;
    };
 //---------------------------------------------------------------------------//
 //---------------------------------------------------------------------------//
    struct TextureUploadData
    {
      TextureUploadData()
        : myData(nullptr), myRowSizeBytes(0u), myTotalSizeBytes(0u), myPixelSizeBytes(0u), mySliceSizeBytes(0u)
      {}
    
      TextureUploadData(const TextureParams& someParams);
      
      uint8* myData;
      uint64 myPixelSizeBytes;
      uint64 myRowSizeBytes;
      uint64 mySliceSizeBytes;
      uint64 myTotalSizeBytes;
    };
//---------------------------------------------------------------------------//
    enum class GpuBufferUsage {
      CONSTANT_BUFFER = (1 << 0),
      VERTEX_BUFFER = (1 << 1),
      INDEX_BUFFER = (1 << 2),
      DRAW_INDIRECT_BUFFER = (1 << 3),
      DISPATCH_INDIRECT_BUFFER = (1 << 4),
      RESOURCE_BUFFER = (1 << 5),
      RESOURCE_BUFFER_RW = (1 << 6),
      RESOURCE_BUFFER_LARGE = (1 << 7),
      RESOURCE_BUFFER_LARGE_RW = (1 << 8),
    };
 //---------------------------------------------------------------------------//
    struct GpuBufferCreationParams {
      GpuBufferCreationParams() : uNumElements(0u), myInternalRefIndex(~0u), bIsMultiBuffered(false), 
        uElementSizeBytes(0u), myUsageFlags(0u), uAccessFlags(0u) {}

      uint uNumElements;
      uint uElementSizeBytes;
      uint uAccessFlags;
      uint myInternalRefIndex;
      uint myUsageFlags;
      bool bIsMultiBuffered;
    };
 //---------------------------------------------------------------------------//
    enum class VertexSemantics {
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
    enum class SamplerFilterMode {
      NEAREST = 0,
      BILINEAR,
      TRILINEAR,
      ANISOTROPIC,

      NUM
    };
//---------------------------------------------------------------------------//
    enum class SamplerAddressMode {
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
  } // end of namespace Rendering 
  }  // end of namespace Fancy
//---------------------------------------------------------------------------//