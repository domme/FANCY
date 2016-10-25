#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "FenceDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  FenceDX12::FenceDX12()
    : myGpuFence(nullptr)
    , myIsDoneEvent(nullptr)
    , myCurrWaitingOnVal(0u)
    , myLastCompletedVal(0u)
  {

  }
//---------------------------------------------------------------------------//
  FenceDX12::FenceDX12(ID3D12Device* aDevice, const String& aName) 
    : myGpuFence(nullptr)
    , myIsDoneEvent(nullptr)
    , myCurrWaitingOnVal(0u)
    , myLastCompletedVal(0u)
  {
    Init(aDevice, aName);
  }
//---------------------------------------------------------------------------//
  void FenceDX12::Init(ID3D12Device* aDevice, const String& aName)
  {
    CheckD3Dcall(aDevice->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myGpuFence)));

    myIsDoneEvent = CreateEventEx(nullptr, nullptr, 0u, EVENT_ALL_ACCESS);
    ASSERT(myIsDoneEvent != nullptr);
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
//---------------------------------------------------------------------------//
} } }

#endif
