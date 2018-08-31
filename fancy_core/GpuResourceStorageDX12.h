#pragma once

#include "CommandListType.h"
#include "DX12Prerequisites.h"
#include "GpuResourceStorage.h"
#include "GpuMemoryAllocatorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceStorageDX12 : public GpuResourceStorage
  {
  public:
    GpuResourceStorageDX12()
      : myReadState((D3D12_RESOURCE_STATES) 0u)
      , myState(0u)
      , myLastCommandListType(CommandListType::Graphics)
    { }
   
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
    GpuMemoryAllocationDX12 myGpuMemory;

    // TODO: Multi-plane and subresource state tracking
    D3D12_RESOURCE_STATES myReadState;
    uint myState;
    CommandListType myLastCommandListType;
  };
//---------------------------------------------------------------------------//
}