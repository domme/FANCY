#include "AdapterDX12.h"


namespace Fancy { namespace Rendering { namespace DX12 {
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
  D3D12_RESOURCE_STATES Adapter::toNativeType(const GpuResourceState& aGeneralType)
  {
    switch (aGeneralType)
    {
    case GpuResourceState::RESOURCE_STATE_COMMON: return D3D12_RESOURCE_STATE_COMMON;
    case GpuResourceState::RESOURCE_STATE_GENERIC_READ: return D3D12_RESOURCE_STATE_GENERIC_READ;
    case GpuResourceState::RESOURCE_STATE_DEPTH_WRITE: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case GpuResourceState::RESOURCE_STATE_DEPTH_READ: return D3D12_RESOURCE_STATE_DEPTH_READ;
    case GpuResourceState::RESOURCE_STATE_PRESENT: return D3D12_RESOURCE_STATE_PRESENT;
    case GpuResourceState::RESOURCE_STATE_COPY_DEST: return D3D12_RESOURCE_STATE_COPY_DEST;
    case GpuResourceState::RESOURCE_STATE_COPY_SRC: return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case GpuResourceState::RESOURCE_STATE_RENDER_TARGET: return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case GpuResourceState::NONE: return static_cast<D3D12_RESOURCE_STATES>(~0u);
    default: ASSERT(false, "Missing native values"); return D3D12_RESOURCE_STATE_COMMON;
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
} } }


