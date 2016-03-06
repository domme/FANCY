#pragma once

#if defined (RENDERER_DX12)

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class FenceDX12
  {
  public:
    FenceDX12();
    explicit FenceDX12(ID3D12Device* aDevice, const String& aName);
    ~FenceDX12();

    void Init(ID3D12Device* aDevice, const String& aName);
    
    uint64 GetCurrWaitingFenceVal() const { return myCurrWaitingOnVal; }
    uint64 GetLastCompletedFenceVal() const { return myLastCompletedVal; }
    uint64 signal(ID3D12CommandQueue* aCommandQueue, uint64 aFenceVal = ~0u);
    void wait();
    bool IsDone(uint64 anOtherFenceVal);
   
  protected:
    HANDLE myIsDoneEvent;
    Microsoft::WRL::ComPtr<ID3D12Fence> myFence;
    uint64 myCurrWaitingOnVal;   // The last signaled value, which might not yet be completed
    uint64 myLastCompletedVal;  // The last completed value
    String myName;

  };
//---------------------------------------------------------------------------//
} } }

#endif

