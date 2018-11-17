#pragma once

#include "CommandListType.h"
#include "DX12Prerequisites.h"
#include "GpuMemoryAllocatorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceDataDX12
  {
  public:
    GpuResourceDataDX12()
      : myReadState((D3D12_RESOURCE_STATES) 0u)
      , myCanChangeStates(true)
      , myAllSubresourcesInSameState(true)
      , mySubresourceStates{ (D3D12_RESOURCE_STATES)0u }
      , mySubresourceContexts{ CommandListType::Graphics }
    { }
   
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
    GpuMemoryAllocationDX12 myGpuMemory;

    D3D12_RESOURCE_STATES myReadState;
    bool myCanChangeStates;
    bool myAllSubresourcesInSameState;
    DynamicArray<D3D12_RESOURCE_STATES> mySubresourceStates;
    DynamicArray<CommandListType> mySubresourceContexts;
  };
//---------------------------------------------------------------------------//
}