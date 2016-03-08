#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorDX12
  {
  public:
    DescriptorDX12()
      : myCpuHandle()
      , myGpuHandle()
    {
      myCpuHandle.ptr = static_cast<uint64>(-1);
      myGpuHandle.ptr = static_cast<uint64>(-1);
    }

    explicit DescriptorDX12(D3D12_CPU_DESCRIPTOR_HANDLE aCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE aGpuHandle)
      : myCpuHandle(aCpuHandle)
      , myGpuHandle(aGpuHandle)
    {}

    D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return myCpuHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return myGpuHandle; }

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHandle;
  };
//---------------------------------------------------------------------------//
} } }