#pragma once

#include "DX12Prerequisites.h"
#include "GpuResourceStorage.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceStorageDX12 : public GpuResourceStorage
  {
  public:
    GpuResourceStorageDX12() = default;
   
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
    GpuMemoryAllocationDX12 myGpuMemory;

    uint myState;


  };
//---------------------------------------------------------------------------//
}