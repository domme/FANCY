#pragma once

#include "DX12Prerequisites.h"
#include "GpuMemoryAllocationDX12.h"
#include "ResourceBarrierStatesDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  //---------------------------------------------------------------------------//
  struct GpuResourceDataDX12 {
    Microsoft::WRL::ComPtr< ID3D12Resource > myResource;
    GpuMemoryAllocationDX12                  myGpuMemory;
    mutable GpuResourceBarrierDataDX12       myHazardData;
    // TODO: Double-check if this is still needed after transitioning to enhanced barriers. If not, remove it and simplify the code that checks it.
    bool myCanChangeStates = true;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy

#endif