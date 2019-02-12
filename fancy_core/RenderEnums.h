#pragma once

namespace Fancy
{
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
    BLEND_FACTOR,// DX11-only
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
    VERTEX = 0,
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
    VERTEX = 0x01,
    FRAGMENT = 0x02,
    GEOMETRY = 0x04,
    TESS_HULL = 0x08,
    TESS_DOMAIN = 0x10,
    COMPUTE = 0x20,

    ALL = 0xFF
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
    TO_READ_GRAPHICS_COMPUTE, // make a resource written by default/compute contexts readable by any default/compute context
    TO_READ_WRITE_DMA,				// make a resource written by any context read/writable by DMA context
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
  enum class CommandListType
  {
    Graphics = 0,
    Compute,
    DMA,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class QueryType
  {
    TIMESTAMP = 0,
    OCCLUSION,

    NUM
  };
//---------------------------------------------------------------------------//
}