#pragma once

#include "RendererPrerequisites.h"

#if defined (RENDERER_DX12)

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class RendererDX12;
  class RenderContextDX12;
//---------------------------------------------------------------------------//
  class GpuResourceDX12
  {
    friend class RendererDX12;
    friend class RenderContextDX12;

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

#endif  // RENDERER_DX12