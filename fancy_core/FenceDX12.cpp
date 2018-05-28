#include "StdAfx.h"

#include "FenceDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  FenceDX12::FenceDX12(CommandListType aCommandListType)
    : Fence(aCommandListType)
    , myFence(nullptr)
  {
    myCommandQueue = RenderCore::GetPlatformDX12()->ourCommandQueues[(uint)myCommandListType].Get();

    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->
      CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence)));
  }
//---------------------------------------------------------------------------//
  void FenceDX12::Signal(uint64 aFenceVal /* = 0u */)
  {
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
    myLastCompletedVal = glm::max(myLastCompletedVal, myFence->GetCompletedValue());
    return anOtherFenceVal <= myLastCompletedVal;
  }
//---------------------------------------------------------------------------//
  FenceDX12::~FenceDX12()
  {
  
  }

  bool FenceDX12::IsDone()
  {
  }

  void FenceDX12::Wait()
  {
  }
//---------------------------------------------------------------------------//
}
