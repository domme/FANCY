#include "StdAfx.h"

#include "FenceDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FenceDX12::FenceDX12()
    : myGpuFence(nullptr)
  {
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->
      CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myGpuFence)));
  }
//---------------------------------------------------------------------------//
  FenceDX12::FenceDX12(const String& aName) 
    : Fence(aName)
    , myGpuFence(nullptr)
  {
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->
      CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myGpuFence)));
  }
//---------------------------------------------------------------------------//
  uint64 FenceDX12::signal(ID3D12CommandQueue* aCommandQueue, uint64 aFenceVal /* = ~0u */)
  {
    if (aFenceVal != ~0u && aFenceVal > myCurrWaitingOnVal)
      myCurrWaitingOnVal = aFenceVal;
    else
      ++myCurrWaitingOnVal;

    aCommandQueue->Signal(myGpuFence.Get(), myCurrWaitingOnVal);
    return myCurrWaitingOnVal;
  }
//---------------------------------------------------------------------------//
  void FenceDX12::wait()
  {
    if (IsDone(myCurrWaitingOnVal))
      return;

    myGpuFence->SetEventOnCompletion(myCurrWaitingOnVal, myIsDoneEvent);
    WaitForSingleObject(myIsDoneEvent, INFINITE);
    myLastCompletedVal = myCurrWaitingOnVal;
  }
//---------------------------------------------------------------------------//
  bool FenceDX12::IsDone(uint64 anOtherFenceVal)
  {
    // The fast path: the other fence-value is passed if the last completed val is greater/equal
    if (anOtherFenceVal <= myLastCompletedVal)
      return true;

    // Otherwise, we can't be sure and need to fetch the fence's value (potentially expensive..)
    myLastCompletedVal = glm::max(myLastCompletedVal, myGpuFence->GetCompletedValue());

    return anOtherFenceVal <= myLastCompletedVal;
  }
//---------------------------------------------------------------------------//
  FenceDX12::~FenceDX12()
  {
  
  }

  void FenceDX12::Wait()
  {
  }
//---------------------------------------------------------------------------//
}
