#include "fancy_core_precompile.h"
#include "AdapterDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
	D3D12_FILL_MODE Adapter::toNativeType(const FillMode& generalType) 
	{
		switch (generalType) 
		{
			case FillMode::WIREFRAME: return D3D12_FILL_MODE_WIREFRAME;
			case FillMode::SOLID:     return D3D12_FILL_MODE_SOLID;
			default: ASSERT(false, "Missing native values"); return D3D12_FILL_MODE_SOLID;
		}
	}
//---------------------------------------------------------------------------//
	D3D12_CULL_MODE Adapter::toNativeType(const CullMode& generalType) 
	{
		switch (generalType) {
			case CullMode::NONE:  return D3D12_CULL_MODE_NONE; 
			case CullMode::FRONT: return D3D12_CULL_MODE_FRONT;
			case CullMode::BACK:  return D3D12_CULL_MODE_BACK;
			default: ASSERT(false, "Missing native values"); return D3D12_CULL_MODE_NONE;
		}
	}
//---------------------------------------------------------------------------//
  D3D12_PRIMITIVE_TOPOLOGY_TYPE Adapter::ResolveTopologyType(const TopologyType& aGeneralType)
  {
    switch (aGeneralType)
    {
    case TopologyType::TRIANGLE_LIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case TopologyType::LINES: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    default: ASSERT(false, "Missing native values"); return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_PRIMITIVE_TOPOLOGY Adapter::ResolveTopology(const TopologyType& aGeneralType)
  {
    switch (aGeneralType)
    {
    case TopologyType::TRIANGLE_LIST: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case TopologyType::LINES: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    default: ASSERT(false, "Missing native values"); return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_COMMAND_LIST_TYPE Adapter::ResolveCommandListType(const CommandListType& aGeneralType)
  {
    switch (aGeneralType)
    {
    case CommandListType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case CommandListType::DMA: return D3D12_COMMAND_LIST_TYPE_COPY;
    default: ASSERT(false, "Missing native values"); return D3D12_COMMAND_LIST_TYPE_DIRECT;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_HEAP_FLAGS Adapter::ResolveHeapFlags(GpuMemoryType aType)
  {
    switch (aType)
    {
    case GpuMemoryType::BUFFER: return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
    case GpuMemoryType::TEXTURE: return D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
    case GpuMemoryType::RENDERTARGET: return D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
    default:
      ASSERT(false, "Missing implementation"); return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
    }
  }
//---------------------------------------------------------------------------//
  GpuMemoryType Adapter::ResolveGpuMemoryType(D3D12_HEAP_FLAGS aHeapFlags)
  {
    switch (aHeapFlags)
    {
    case D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS: return GpuMemoryType::BUFFER;
    case D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES: return GpuMemoryType::TEXTURE;
    case D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES: return GpuMemoryType::RENDERTARGET;
    default:
      ASSERT(false, "Missing implementation"); return GpuMemoryType::BUFFER;
    }
  }
//---------------------------------------------------------------------------//
  CpuMemoryAccessType Adapter::ResolveGpuMemoryAccessType(D3D12_HEAP_TYPE aHeapType)
  {
    switch (aHeapType)
    {
    case D3D12_HEAP_TYPE_DEFAULT: return CpuMemoryAccessType::NO_CPU_ACCESS;
    case D3D12_HEAP_TYPE_UPLOAD: return CpuMemoryAccessType::CPU_WRITE;
    case D3D12_HEAP_TYPE_READBACK: return CpuMemoryAccessType::CPU_READ;
    default:
      ASSERT(false, "Missing implementation"); return CpuMemoryAccessType::NO_CPU_ACCESS;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_RESOURCE_DIMENSION Adapter::ResolveResourceDimension(GpuResourceDimension aDimension, bool& aCubeMapOut, bool& anArrayOut)
  {
    aCubeMapOut = false;
    anArrayOut = false;

    switch (aDimension) 
    { 
      case GpuResourceDimension::UNKONWN: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
      case GpuResourceDimension::BUFFER: return D3D12_RESOURCE_DIMENSION_BUFFER;
      case GpuResourceDimension::TEXTURE_1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
      case GpuResourceDimension::TEXTURE_2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      case GpuResourceDimension::TEXTURE_3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
      case GpuResourceDimension::TEXTURE_CUBE:        aCubeMapOut = true; return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      case GpuResourceDimension::TEXTURE_1D_ARRAY:    anArrayOut = true;  return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
      case GpuResourceDimension::TEXTURE_2D_ARRAY:    anArrayOut = true;  return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      case GpuResourceDimension::TEXTURE_CUBE_ARRAY:  aCubeMapOut = true; anArrayOut = true; return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      default: 
        ASSERT(false, "Missing implementation"); return D3D12_RESOURCE_DIMENSION_UNKNOWN;
    }
  }
//---------------------------------------------------------------------------//
  D3D12_QUERY_TYPE Adapter::ResolveQueryType(GpuQueryType aQueryType)
  {
    switch(aQueryType) { 
      case GpuQueryType::TIMESTAMP: return D3D12_QUERY_TYPE_TIMESTAMP;
      case GpuQueryType::OCCLUSION: return D3D12_QUERY_TYPE_OCCLUSION;
      default: 
        ASSERT(false, "Missing implementation"); return D3D12_QUERY_TYPE_TIMESTAMP;
    }
}
//---------------------------------------------------------------------------//
  D3D12_QUERY_HEAP_TYPE Adapter::ResolveQueryHeapType(GpuQueryType aQueryType)
  {
    switch (aQueryType) {
    case GpuQueryType::TIMESTAMP: return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    case GpuQueryType::OCCLUSION: return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
    default:
      ASSERT(false, "Missing implementation"); return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    }
  }
//---------------------------------------------------------------------------//
	D3D12_BLEND Adapter::toNativeType(const BlendFactor& generalType) 
	{
		switch (generalType) {
			case BlendFactor::ZERO:                return D3D12_BLEND_ZERO;
			case BlendFactor::ONE:                 return D3D12_BLEND_ONE;
			case BlendFactor::SRC_COLOR:           return D3D12_BLEND_SRC_COLOR;
			case BlendFactor::INV_SRC_COLOR:       return D3D12_BLEND_INV_SRC_COLOR;
			case BlendFactor::SRC_ALPHA:           return D3D12_BLEND_SRC_ALPHA;
			case BlendFactor::INV_SRC_ALPHA:       return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendFactor::DEST_ALPHA:          return D3D12_BLEND_DEST_ALPHA;
			case BlendFactor::INV_DEST_ALPHA:      return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendFactor::DEST_COLOR:          return D3D12_BLEND_DEST_COLOR;
			case BlendFactor::INV_DEST_COLOR:      return D3D12_BLEND_INV_DEST_COLOR;
			case BlendFactor::SRC_ALPHA_CLAMPED:   return D3D12_BLEND_SRC_ALPHA_SAT;
      case BlendFactor::SRC1_COLOR:          return D3D12_BLEND_SRC1_COLOR;
      case BlendFactor::INV_SRC1_COLOR:      return D3D12_BLEND_INV_SRC1_COLOR;
      case BlendFactor::SRC1_ALPHA:          return D3D12_BLEND_SRC1_ALPHA;
      case BlendFactor::INV_SRC1_ALPHA:      return D3D12_BLEND_INV_SRC1_ALPHA;
      case BlendFactor::CONSTANT_COLOR:      return D3D12_BLEND_BLEND_FACTOR;
			case BlendFactor::INV_CONSTANT_COLOR:  return D3D12_BLEND_INV_BLEND_FACTOR;
			
			default: ASSERT(false, "Missing native values"); return D3D12_BLEND_ZERO;
		}
	}
//---------------------------------------------------------------------------//
	D3D12_BLEND_OP Adapter::toNativeType(const BlendOp& generalType) 
	{
		switch (generalType) {
			case BlendOp::ADD:            return D3D12_BLEND_OP_ADD;
			case BlendOp::SUBTRACT:       return D3D12_BLEND_OP_SUBTRACT;
			case BlendOp::REV_SUBTRACT:   return D3D12_BLEND_OP_REV_SUBTRACT;
			case BlendOp::MIN:            return D3D12_BLEND_OP_MIN;
			case BlendOp::MAX:            return D3D12_BLEND_OP_MAX;
			default: ASSERT(false, "Missing implementation"); return D3D12_BLEND_OP_ADD;
		}
	}
//---------------------------------------------------------------------------//
}


#endif