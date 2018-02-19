#pragma once

#include "FancyCorePrerequisites.h"
#include "DX12Prerequisites.h"
#include "Descriptor.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorDX12 : public Descriptor
  {
  public:
    DescriptorDX12()
      : myCpuHandle()
      , myGpuHandle()
      , myHeapType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
    {
      myCpuHandle.ptr = static_cast<uint64>(0);
      myGpuHandle.ptr = static_cast<uint64>(0);
    }
       
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHandle;
    D3D12_DESCRIPTOR_HEAP_TYPE myHeapType;
  };
//---------------------------------------------------------------------------//
} } }