#pragma once

#include "DX12Prerequisites.h"
#include "RenderEnums.h"

namespace Fancy {
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
    static D3D12_PRIMITIVE_TOPOLOGY_TYPE ResolveTopologyType(const TopologyType& aGeneralType);
    static D3D12_PRIMITIVE_TOPOLOGY ResolveTopology(const TopologyType& aGeneralType);
    static D3D12_COMMAND_LIST_TYPE ResolveCommandListType(const CommandListType& aGeneralType);
	  static D3D12_HEAP_FLAGS ResolveHeapFlags(GpuMemoryType aType);
    static GpuMemoryType ResolveGpuMemoryType(D3D12_HEAP_FLAGS aHeapFlags);
    static CpuMemoryAccessType ResolveGpuMemoryAccessType(D3D12_HEAP_TYPE aHeapType);
    static D3D12_RESOURCE_DIMENSION ResolveResourceDimension(GpuResourceDimension aDimension, bool& aCubeMapOut, bool& anArrayOut);
    static D3D12_QUERY_TYPE ResolveQueryType(GpuQueryType aQueryType);
    static D3D12_QUERY_HEAP_TYPE ResolveQueryHeapType(GpuQueryType aQueryType);
	};
//---------------------------------------------------------------------------//
}
