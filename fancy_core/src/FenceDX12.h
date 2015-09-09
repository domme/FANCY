#pragma once

#if defined (RENDERER_DX12)

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {

  class FenceDX12
  {
  public:
    explicit FenceDX12();
    ~FenceDX12();

    void init(ID3D12Device* aDevice, const String& aName);
    void signal(ID3D12CommandQueue* aCommandQueue);
    void wait();
   
  protected:
    friend class RendererDX12;

    HANDLE myIsDoneEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
    uint myFenceVal;
    String myName;
  };

} } }

#endif

