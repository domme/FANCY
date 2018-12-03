#include "AdapterDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
	D3D12_COMPARISON_FUNC Adapter::toNativeType(const CompFunc& generalType) 
	{	
		static D3D12_COMPARISON_FUNC ourTranslationTable[] =
		{
			D3D12_COMPARISON_FUNC_NEVER,
			D3D12_COMPARISON_FUNC_LESS,
			D3D12_COMPARISON_FUNC_EQUAL,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_COMPARISON_FUNC_GREATER,
			D3D12_COMPARISON_FUNC_NOT_EQUAL,
			D3D12_COMPARISON_FUNC_GREATER_EQUAL,
			D3D12_COMPARISON_FUNC_NEVER
		};
		STATIC_ASSERT(ARRAY_LENGTH(ourTranslationTable) == (uint)CompFunc::NUM, "Missing DX12 implementations");

		return ourTranslationTable[(uint)generalType];
	}
//---------------------------------------------------------------------------//
	D3D12_STENCIL_OP Adapter::toNativeType(const StencilOp& generalType) 
	{
    switch (generalType)
    {
      case StencilOp::KEEP: return D3D12_STENCIL_OP_KEEP;
      case StencilOp::ZERO: return D3D12_STENCIL_OP_ZERO;
      case StencilOp::REPLACE: return D3D12_STENCIL_OP_REPLACE;
      case StencilOp::INCREMENT_CLAMP: return D3D12_STENCIL_OP_INCR_SAT;
      case StencilOp::DECREMENT_CLAMP: return D3D12_STENCIL_OP_DECR_SAT;
      case StencilOp::INVERT: return D3D12_STENCIL_OP_INVERT;
      case StencilOp::INCEMENT_WRAP: return D3D12_STENCIL_OP_INCR;
      case StencilOp::DECREMENT_WRAP: return D3D12_STENCIL_OP_DECR;
      default: ASSERT(false); return D3D12_STENCIL_OP_KEEP;
    }
	}
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
	D3D12_BLEND Adapter::toNativeType(const BlendInput& generalType) 
	{
		switch (generalType) {
			case BlendInput::ZERO:                return D3D12_BLEND_ZERO;
			case BlendInput::ONE:                 return D3D12_BLEND_ONE;
			case BlendInput::SRC_COLOR:           return D3D12_BLEND_SRC_COLOR;
			case BlendInput::INV_SRC_COLOR:       return D3D12_BLEND_INV_SRC_COLOR;
			case BlendInput::SRC_ALPHA:           return D3D12_BLEND_SRC_ALPHA;
			case BlendInput::INV_SRC_ALPHA:       return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendInput::DEST_ALPHA:          return D3D12_BLEND_DEST_ALPHA;
			case BlendInput::INV_DEST_ALPHA:      return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendInput::DEST_COLOR:          return D3D12_BLEND_DEST_COLOR;
			case BlendInput::INV_DEST_COLOR:      return D3D12_BLEND_INV_DEST_COLOR;
			case BlendInput::SRC_ALPHA_CLAMPED:   return D3D12_BLEND_SRC_ALPHA_SAT;
			case BlendInput::CONSTANT_COLOR:
			case BlendInput::CONSTANT_ALPHA:
			case BlendInput::BLEND_FACTOR:        return D3D12_BLEND_BLEND_FACTOR;
			case BlendInput::INV_CONSTANT_COLOR:
			case BlendInput::INV_CONSTANT_ALPHA:
			case BlendInput::INV_BLEND_FACTOR:    return D3D12_BLEND_INV_BLEND_FACTOR;
			case BlendInput::SRC1_COLOR:          return D3D12_BLEND_SRC1_COLOR;
			case BlendInput::INV_SRC1_COLOR:      return D3D12_BLEND_INV_SRC1_COLOR;
			case BlendInput::SRC1_ALPHA:          return D3D12_BLEND_SRC1_ALPHA;
			case BlendInput::INV_SRC1_ALPHA:      return D3D12_BLEND_INV_SRC1_ALPHA;
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


