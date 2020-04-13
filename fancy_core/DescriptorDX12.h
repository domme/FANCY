#pragma once

#include "DX12Prerequisites.h"

#if FANCY_ENABLE_DX12

namespace Fancy { 
//---------------------------------------------------------------------------//
  class DescriptorDX12
  {
  public:
    DescriptorDX12()
      : myCpuHandle{UINT_MAX}
      , myGpuHandle{UINT_MAX}
      , myHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES)
      , myIsManagedByAllocator(false)
    {
    }

    DescriptorDX12(D3D12_CPU_DESCRIPTOR_HANDLE aCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE aGpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType, bool aIsManagedByAllocator)
      : myCpuHandle(aCpuHandle)
      , myGpuHandle(aGpuHandle)
      , myHeapType(aHeapType)
      , myIsManagedByAllocator(aIsManagedByAllocator)
    {

    }
       
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHandle;
    D3D12_DESCRIPTOR_HEAP_TYPE myHeapType;
    bool myIsManagedByAllocator;
  };
//---------------------------------------------------------------------------//
}

#endif