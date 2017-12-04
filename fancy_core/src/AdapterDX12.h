#pragma once

#include "DX12Prerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
	class Adapter
	{
		public:
		static D3D12_BLEND_OP toNativeType(const BlendOp& generalType);
		static D3D12_BLEND toNativeType(const BlendInput& generalType);
		static D3D12_COMPARISON_FUNC toNativeType(const CompFunc& generalType);
		static D3D12_STENCIL_OP toNativeType(const StencilOp& generalType);
		static D3D12_FILL_MODE toNativeType(const FillMode& generalType);
		static D3D12_CULL_MODE toNativeType(const CullMode& generalType);
    static D3D12_RESOURCE_STATES toNativeType(const GpuResourceState& aGeneralType);
	};
//---------------------------------------------------------------------------//
} } }
