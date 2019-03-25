#include "fancy_core_precompile.h"
#include "CommandQueueDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "CommandContextDX12.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandQueueDX12::CommandQueueDX12(CommandListType aType)
    : CommandQueue(aType)
    , myLastCompletedFenceVal(myInitialFenceVal)
    , myNextFenceVal(myInitialFenceVal + 1)
    , myFenceCompletedEvent(nullptr)
  {
	  ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

	  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = Adapter::ResolveCommandListType(aType); 
	  CheckD3Dcall(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myQueue)));

    CheckD3Dcall(device->CreateFence(myLastCompletedFenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence)));
  }
//---------------------------------------------------------------------------//
  bool CommandQueueDX12::IsFenceDone(uint64 aFenceVal)
  {
    if (myLastCompletedFenceVal < aFenceVal)
      myLastCompletedFenceVal = glm::max(myLastCompletedFenceVal, myFence->GetCompletedValue());
      
    return myLastCompletedFenceVal >= aFenceVal;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::SignalAndIncrementFence()
  {
    CheckD3Dcall(myQueue->Signal(myFence.Get(), myNextFenceVal));
    return myNextFenceVal++;
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForFence(uint64 aFenceVal)
  {
    if (IsFenceDone(aFenceVal))
      return;

    CheckD3Dcall(myFence->SetEventOnCompletion(aFenceVal, myFenceCompletedEvent));
    WaitForSingleObject(myFenceCompletedEvent, INFINITE);
    myLastCompletedFenceVal = glm::max(myLastCompletedFenceVal, aFenceVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForIdle()
  {
    WaitForFence(SignalAndIncrementFence());
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::ExecuteContext(CommandContext* aContext, bool aWaitForCompletion /* = false */)
  {
    ASSERT(aContext->GetType() == myType);

    CommandContextDX12* contextDx12 = (CommandContextDX12*)aContext;
    contextDx12->CloseCommandList();

    ID3D12CommandList* cmdList = contextDx12->myCommandList;
    myQueue->ExecuteCommandLists(1, &cmdList);

    const uint64 fenceVal = SignalAndIncrementFence();
    contextDx12->Reset(fenceVal);

    if (aWaitForCompletion)
      WaitForFence(fenceVal);

    return fenceVal;
  }
//---------------------------------------------------------------------------//
}