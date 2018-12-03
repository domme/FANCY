#pragma once

#include "CommandListType.h"
#include "DX12Prerequisites.h"
#include "GpuMemoryAllocatorDX12.h"
#include "GpuResource.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuHazardDataDX12 : GpuHazardData
  {
    GpuHazardDataDX12()
      : myReadState((D3D12_RESOURCE_STATES)0u)
      , mySubresourceStates{ (D3D12_RESOURCE_STATES)0u }
    { }

    D3D12_RESOURCE_STATES myReadState;
    DynamicArray<D3D12_RESOURCE_STATES> mySubresourceStates;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceDataDX12
  {
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
    GpuMemoryAllocationDX12 myGpuMemory;
  };
//---------------------------------------------------------------------------//
}