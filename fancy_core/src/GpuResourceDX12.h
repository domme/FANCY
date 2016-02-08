#pragma once

#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuResourceDX12
  {
  public:
    GpuResourceDX12()
      : myUsageState(D3D12_RESOURCE_STATE_COMMON)
      , myTransitioningState(static_cast<D3D12_RESOURCE_STATES>(~0))
      , myGpuVirtualAddress(0u)
    {}

    ComPtr<ID3D12Resource> GetResource() const  { return myResource; }
    D3D12_RESOURCE_STATES GetUsageState() const { return myUsageState; }
    D3D12_RESOURCE_STATES GetTransitioningState() const { return myTransitioningState; }
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return myGpuVirtualAddress; }

  protected:
    ComPtr<ID3D12Resource> myResource;
    D3D12_RESOURCE_STATES myUsageState;
    D3D12_RESOURCE_STATES myTransitioningState;
    D3D12_GPU_VIRTUAL_ADDRESS myGpuVirtualAddress;
  };
//---------------------------------------------------------------------------//
} } }

#endif  // RENDERER_DX12