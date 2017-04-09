#pragma once

#include "RendererPrerequisites.h"
#include "DX12Prerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RenderOutputDX12;
  class RenderContextDX12;
//---------------------------------------------------------------------------//
  class GpuResourceDX12
  {
    friend class RenderOutputDX12;
    friend class RenderContextDX12;
    friend class CommandContextDX12;

  public:
    GpuResourceDX12()
      : myUsageState(D3D12_RESOURCE_STATE_COMMON)
      , myTransitioningState(static_cast<D3D12_RESOURCE_STATES>(~0))
    {
    }

    ID3D12Resource* GetResource() const  { return myResource.Get(); }
    D3D12_RESOURCE_STATES GetUsageState() const { return myUsageState; }
    D3D12_RESOURCE_STATES GetTransitioningState() const { return myTransitioningState; }
    D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return myResource->GetGPUVirtualAddress(); }

  protected:
    ComPtr<ID3D12Resource> myResource;
    D3D12_RESOURCE_STATES myUsageState;
    D3D12_RESOURCE_STATES myTransitioningState;

    void Reset()
    {
      myResource = nullptr;
      myUsageState = static_cast<D3D12_RESOURCE_STATES>(~0);
      myTransitioningState = static_cast<D3D12_RESOURCE_STATES>(~0);
    }
  };
//---------------------------------------------------------------------------//
} } }