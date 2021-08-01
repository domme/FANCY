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
  enum class BlendFactor
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
    SRC1_COLOR,
    INV_SRC1_COLOR,
    SRC1_ALPHA,
    INV_SRC1_ALPHA,
    CONSTANT_COLOR,
    INV_CONSTANT_COLOR,

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
  enum class LogicOp
  {
    CLEAR = 0,
    AND,
    AND_REVERSE,
    COPY,
    AND_INVERTED,
    NO_OP,
    XOR,
    OR,
    NOR,
    EQUIVALENT,
    INVERT,
    OR_REVERSE,
    COPY_INVERTED,
    OR_INVERTED,
    NAND,
    SET,

    NUM
  };
//---------------------------------------------------------------------------//
  enum ShaderStage
  {
    SHADERSTAGE_VERTEX = 0,
    SHADERSTAGE_FRAGMENT,
    SHADERSTAGE_GEOMETRY,
    SHADERSTAGE_TESS_HULL,
    SHADERSTAGE_TESS_DOMAIN,
    SHADERSTAGE_COMPUTE,
    SHADERSTAGE_RAYGEN,
    SHADERSTAGE_MISS,
    SHADERSTAGE_INTERSECTION,
    SHADERSTAGE_ANYHIT,
    SHADERSTAGE_CLOSEST_HIT,
    
    SHADERSTAGE_NUM,
    SHADERSTAGE_NUM_RASTERIZATION = SHADERSTAGE_TESS_DOMAIN + 1 - SHADERSTAGE_VERTEX,
    SHADERSTAGE_NUM_RAYTRACING = SHADERSTAGE_CLOSEST_HIT + 1 - SHADERSTAGE_RAYGEN,
    SHADERSTAGE_NONE = SHADERSTAGE_NUM
  };
//---------------------------------------------------------------------------//
  inline bool IsRaytracingStage(ShaderStage aStage)
  {
    return aStage >= SHADERSTAGE_RAYGEN && aStage <= SHADERSTAGE_CLOSEST_HIT;
  }
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
  enum class GpuResourceType
  {
    TEXTURE = 0,
    BUFFER
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
    TEXTURE_CUBE_ARRAY,
    NUM
  };
//---------------------------------------------------------------------------//
  enum class GpuBufferBindFlags
  {
    NONE = 0,
    CONSTANT_BUFFER = 1 << 0,
    VERTEX_BUFFER = 1 << 1,
    INDEX_BUFFER = 1 << 2,
    SHADER_BUFFER = 1 << 3,
    RAYTRACING_BVH_BUILD_INPUT = 1 << 4,
    RAYTRACING_BVH_STORAGE = 1 << 5,
    RAYTRACING_BVH_BINDING_TABLE = 1 << 6,

    ALL = ~0
  };
//---------------------------------------------------------------------------//
  enum class GpuBufferUsage
  {
    STAGING_UPLOAD = 0,
    STAGING_READBACK,
    CONSTANT_BUFFER,
    VERTEX_BUFFER,
    INDEX_BUFFER,
    SHADER_BUFFER,
  };
//---------------------------------------------------------------------------//
  enum class GpuBufferViewType
  {
    CONSTANT = 0,
    STRUCTURED,
    RAW,
    TYPED
  };
//---------------------------------------------------------------------------//
  enum class VertexAttributeSemantic
  {
    NONE = 0,

    POSITION,
    NORMAL,
    TANGENT,
    BINORMAL,
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
    CLAMP_EDGE = 0,
    MIRROR_CLAMP_EDGE,
    CLAMP_BORDER,
    REPEAT,
    MIRROR_REPEAT,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class SamplerBorderColor
  {
    FLOAT_TRANSPARENT_BLACK = 0,
    INT_TRANSPARENT_BLACK,
    FLOAT_OPAQUE_BLACK,
    INT_OPAQUE_BLACK,
    FLOAT_OPAQUE_WHITE,
    INT_OPAQUE_WHITE,

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

    NUM,
    UNKNOWN,
    SHARED_READ
  };
//---------------------------------------------------------------------------//
  enum class GpuQueryType
  {
    TIMESTAMP = 0,
    OCCLUSION,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class SyncMode
  {
    BLOCKING = 0,
    ASYNC,

    NUM
  };
//---------------------------------------------------------------------------//
  enum ShaderModel
  {
    SM_6_0 = 0,
    SM_6_1,
    SM_6_2,
    SM_6_3,
    SM_6_4,
    SM_LATEST = SM_6_4,
  };
//---------------------------------------------------------------------------//
  enum class ResourceTransition
  {
    TO_SHARED_CONTEXT_READ,  // Transition to all allowed read states on a subresource and mark it for being used simultaneously on different queues
  };
//---------------------------------------------------------------------------//
  enum class ResourceUsageFlags
  {
    SHADER_READ = 1 << 0,
    COPY_SRC = 1 << 1,
    DEPTH_STENCIL_READ = 1 << 2
  };
//---------------------------------------------------------------------------//
  enum class ShaderResourceTransition
  {
    TO_SHADER_READ,
    TO_SHADER_WRITE
  };
//---------------------------------------------------------------------------//
  enum class VertexInputRate
  {
    PER_VERTEX = 0,
    PER_INSTANCE,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class RaytracingBVHType
  {
    BOTTOM_LEVEL = 0,
    TOP_LEVEL,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class RaytracingBVHGeometryType
  {
    TRIANGLES = 0,
    AABBS,
    INSTANCES,

    NUM
  };
//---------------------------------------------------------------------------//
  enum class RaytracingBVHGeometryFlags
  {
    NONE = 0u,
    OPAQUE_GEOMETRY = 1 << 0u,
    NO_DUPLICATE_ANYHIT_INVOCATION = 1 << 1u
  };
//---------------------------------------------------------------------------//
  enum class RaytracingBVHFlags
  {
    NONE = 0u,
    ALLOW_UPDATE = 1 << 0u,
    ALLOW_COMPACTION = 1 << 1u,
    PREFER_FAST_TRACE = 1 << 2u,
    PREFER_FAST_BUILD = 1 << 3u,
    MINIMIZE_MEMORY = 1 << 4u
  };
//---------------------------------------------------------------------------//
  enum RaytracingHitGroupType
  {
    RT_HIT_GROUP_TYPE_TRIANGLES = 0,
    RT_HIT_GROUP_TYPE_PROCEDURAL_GEOMETRY,
    RT_HIT_GROUP_TYPE_NUM,
  };
//---------------------------------------------------------------------------//
  enum RaytracingPipelineFlags
  {
    RT_PIPELINE_FLAG_NONE = 0,
    RT_PIPELINE_FLAG_SKIP_TRIANGLES = 1 << 0u,
    RT_PIPELINE_FLAG_SKIP_PROCEDURAL = 1 << 1u,
  };
//---------------------------------------------------------------------------//
  enum GlobalResourceType
  {
    GLOBAL_RESOURCE_SRV_START = 0,
    GLOBAL_RESOURCE_TEXTURE_1D = GLOBAL_RESOURCE_SRV_START,
    GLOBAL_RESOURCE_TEXTURE_1D_UINT,
    GLOBAL_RESOURCE_TEXTURE_1D_INT,
    GLOBAL_RESOURCE_TEXTURE_2D,
    GLOBAL_RESOURCE_TEXTURE_2D_UINT,
    GLOBAL_RESOURCE_TEXTURE_2D_INT,
    GLOBAL_RESOURCE_TEXTURE_3D,
    GLOBAL_RESOURCE_TEXTURE_3D_UINT,
    GLOBAL_RESOURCE_TEXTURE_3D_INT,
    GLOBAL_RESOURCE_TEXTURE_CUBE,
    GLOBAL_RESOURCE_TEXTURE_CUBE_UINT,
    GLOBAL_RESOURCE_TEXTURE_CUBE_INT,
    GLOBAL_RESOURCE_BUFFER,
    GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE,
    GLOBAL_RESOURCE_SRV_END = GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE + 1,

    GLOBAL_RESOURCE_UAV_START = GLOBAL_RESOURCE_SRV_END,
    GLOBAL_RESOURCE_RWTEXTURE_1D = GLOBAL_RESOURCE_UAV_START,
    GLOBAL_RESOURCE_RWTEXTURE_1D_UINT,
    GLOBAL_RESOURCE_RWTEXTURE_1D_INT,
    GLOBAL_RESOURCE_RWTEXTURE_2D,
    GLOBAL_RESOURCE_RWTEXTURE_2D_UINT,
    GLOBAL_RESOURCE_RWTEXTURE_2D_INT,
    GLOBAL_RESOURCE_RWTEXTURE_3D,
    GLOBAL_RESOURCE_RWTEXTURE_3D_UINT,
    GLOBAL_RESOURCE_RWTEXTURE_3D_INT,
    GLOBAL_RESOURCE_RWBUFFER,
    GLOBAL_RESOURCE_UAV_END = GLOBAL_RESOURCE_RWBUFFER + 1,

    GLOBAL_RESOURCE_SAMPLER = GLOBAL_RESOURCE_UAV_END,
    GLOBAL_RESOURCE_NUM,
    GLOBAL_RESOURCE_NUM_NOSAMPLER = GLOBAL_RESOURCE_SAMPLER
  };
//---------------------------------------------------------------------------//
  


}