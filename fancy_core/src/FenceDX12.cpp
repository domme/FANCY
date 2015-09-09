#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "FenceDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {

  FenceDX12::FenceDX12() :
      myFence(nullptr)
    , myIsDoneEvent(nullptr)
    , myFenceVal(0u)
  {
    
  }

  void FenceDX12::init(ID3D12Device* aDevice, const String& aName)
  {
    HRESULT success = aDevice->CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence));
    ASSERT(success == S_OK);

    myIsDoneEvent = CreateEventEx(nullptr, nullptr, 0u, EVENT_ALL_ACCESS);
    ASSERT(myIsDoneEvent != nullptr);
  }

void FenceDX12::signal(ID3D12CommandQueue* aCommandQueue)
{
  ++myFenceVal;
  aCommandQueue->Signal(myFence.Get(), myFenceVal);
}

void FenceDX12::wait()
{
  if (myFence->GetCompletedValue() < myFenceVal)
  {
    myFence->SetEventOnCompletion(myFenceVal, myIsDoneEvent);
    WaitForSingleObject(myIsDoneEvent, INFINITE);
  }
}

FenceDX12::~FenceDX12()
{
  
}

} } }

#endif
