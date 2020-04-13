#pragma once

#include "DX12Prerequisites.h"
#include "GpuMemoryAllocationDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceDataDX12
  {
    Microsoft::WRL::ComPtr<ID3D12Resource> myResource;
    GpuMemoryAllocationDX12 myGpuMemory;
  };
//---------------------------------------------------------------------------//
}

#endif