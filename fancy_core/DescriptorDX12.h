#pragma once

#include "DX12Prerequisites.h"

namespace Fancy { 
//---------------------------------------------------------------------------//
  class DescriptorDX12
  {
  public:
    DescriptorDX12()
      : myCpuHandle{0u}
      , myGpuHandle{0u}
      , myHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
      , myIsManagedByAllocator(false)
    {
    }
       
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHandle;
    D3D12_DESCRIPTOR_HEAP_TYPE myHeapType;
    bool myIsManagedByAllocator;
  };
//---------------------------------------------------------------------------//
}