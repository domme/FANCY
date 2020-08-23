#pragma once

#include "DX12Prerequisites.h"
#include "GpuMemoryAllocationDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuSubresourceHazardDataDX12
  {
    D3D12_RESOURCE_STATES myStates;
    CommandListType myContext;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardDataDX12
  {
    bool myCanChangeStates = true;
    D3D12_RESOURCE_STATES myReadStates;
    D3D12_RESOURCE_STATES myWriteStates;
    bool myAllSubresourcesSameStates;
    eastl::fixed_vector<GpuSubresourceHazardDataDX12, 16> mySubresources;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceDataDX12
  {
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
    GpuMemoryAllocationDX12 myGpuMemory;
    GpuResourceHazardDataDX12 myHazardData;
  };
//---------------------------------------------------------------------------//
}

#endif