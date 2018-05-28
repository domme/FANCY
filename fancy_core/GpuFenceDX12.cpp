#include "StdAfx.h"

#include "GpuFenceDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuFenceDX12::GpuFenceDX12(CommandListType aCommandListType)
    : GpuFence(aCommandListType)
    , myFence(nullptr)
  {
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->
      CreateFence(0u, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence)));
  }
//---------------------------------------------------------------------------//
  uint64 GpuFenceDX12::SignalAndIncrement()
  {
    GetCommandQueue()->Signal(myFence.Get(), myNextVal);
    return myNextVal++;
  }
//---------------------------------------------------------------------------//
  void GpuFenceDX12::Wait(uint64 aFenceVal)
  {
    if (IsDone(aFenceVal))
      return;

    myFence->SetEventOnCompletion(aFenceVal, myEventHandle);
    WaitForSingleObject(myEventHandle, INFINITE);
    myLastCompletedVal = glm::max(aFenceVal, myLastCompletedVal);
  }
//---------------------------------------------------------------------------//
	ID3D12CommandQueue* GpuFenceDX12::GetCommandQueue() const
	{
		return static_cast<CommandQueueDX12*>(RenderCore::GetCommandQueue(myCommandListType))->myQueue.Get();
	}
//---------------------------------------------------------------------------//
  bool GpuFenceDX12::IsDone(uint64 anOtherFenceVal)
  {
    // The fast path: the other fence-value is passed if the last completed val is greater/equal
    if (anOtherFenceVal <= myLastCompletedVal)
      return true;

    // Otherwise, we can't be sure and need to fetch the fence's value (potentially expensive..)
    myLastCompletedVal = glm::max(myLastCompletedVal, myFence->GetCompletedValue());
    return anOtherFenceVal <= myLastCompletedVal;
  }
//---------------------------------------------------------------------------//
}
