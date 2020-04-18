#include "fancy_core_precompile.h"
#include "CommandQueueDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
//---------------------------------------------------------------------------//
  CommandQueueDX12::CommandQueueDX12(CommandListType aType)
    : CommandQueue(aType)
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
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

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
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

    if (IsFenceDone(aFenceVal))
      return;

    CheckD3Dcall(myFence->SetEventOnCompletion(aFenceVal, myFenceCompletedEvent));
    WaitForSingleObject(myFenceCompletedEvent, INFINITE);
    myLastCompletedFenceVal = glm::max(myLastCompletedFenceVal, aFenceVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForIdle()
  {
    // TODO: Could just be WaitForFence(myNextFenceVal - 1u); ?
    WaitForFence(SignalAndIncrementFence());
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::StallForQueue(const CommandQueue* aCommandQueue)
  {
    const CommandQueueDX12* otherQueue = static_cast<const CommandQueueDX12*>(aCommandQueue);
    if (otherQueue != this)
      CheckD3Dcall(myQueue->Wait(otherQueue->myFence.Get(), otherQueue->myNextFenceVal - 1u));
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::StallForFence(uint64 aFenceVal)
  {
    const CommandQueueDX12* otherQueue = static_cast<const CommandQueueDX12*>(RenderCore::GetCommandQueue(aFenceVal));
    if (otherQueue != this)
      CheckD3Dcall(myQueue->Wait(otherQueue->myFence.Get(), aFenceVal));
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::ExecuteCommandListInternal(CommandList* aCommandList, SyncMode aSyncMode/* = SyncMode::ASYNC*/)
  {
    ASSERT(aCommandList->GetType() == myType);
    ASSERT(aCommandList->IsOpen());

    ResolveResourceHazardData(aCommandList);

    aCommandList->Close();

    CommandListDX12* contextDx12 = (CommandListDX12*)aCommandList;
    ID3D12CommandList* cmdList = contextDx12->myCommandList;
    myQueue->ExecuteCommandLists(1, &cmdList);

    const uint64 fenceVal = SignalAndIncrementFence();
    aCommandList->PostExecute(fenceVal);

    if (aSyncMode == SyncMode::BLOCKING)
      WaitForFence(fenceVal);

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::ExecuteAndResetCommandListInternal(CommandList* aContext, SyncMode aSyncMode)
  {
    const uint64 fenceVal = ExecuteCommandListInternal(aContext, aSyncMode);
    aContext->PreBegin();
    return fenceVal;
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::ResolveResourceHazardData(CommandList* aCommandList)
  {
    DynamicArray<D3D12_RESOURCE_BARRIER> patchingBarriers;

    CommandListDX12* cmdListDx12 = static_cast<CommandListDX12*>(aCommandList);

    for (auto it : cmdListDx12->myLocalHazardData)
    {
      const GpuResource* resource = it.first;

      GpuResourceHazardData& globalHazardData = resource->GetHazardData();
      const CommandListDX12::LocalHazardData& localHazardData = it.second;
      ASSERT(globalHazardData.myDx12Data.mySubresources.size() == localHazardData.mySubresources.size());

      StaticArray<D3D12_RESOURCE_BARRIER, 1024> subresourceTransitions;
      bool canTransitionAllSubresources = true;
      for (uint subIdx = 0u; subIdx < localHazardData.mySubresources.size(); ++subIdx)
      {
        GpuSubresourceHazardDataDX12& globalSubData = globalHazardData.myDx12Data.mySubresources[subIdx];
        const CommandListDX12::SubresourceHazardData& localSubData = localHazardData.mySubresources[subIdx];

        if (localSubData.myIsSharedReadState)
        {
          ASSERT(!localSubData.myWasWritten);
          globalSubData.myContext = CommandListType::SHARED_READ;
        }

        if (!localSubData.myWasUsed || globalSubData.myStates == localSubData.myFirstDstStates)
        {
          canTransitionAllSubresources = false;
          continue;
        }

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.StateBefore = (D3D12_RESOURCE_STATES)globalSubData.myStates;
        barrier.Transition.StateAfter = localSubData.myFirstDstStates;
        barrier.Transition.Subresource = subIdx;
        barrier.Transition.pResource = resource->myNativeData.To<GpuResourceDataDX12*>()->myResource.Get();

        if (subresourceTransitions.Size() > 0
          && (subresourceTransitions.GetLast().Transition.StateBefore != barrier.Transition.StateBefore
          || subresourceTransitions.GetLast().Transition.StateAfter != barrier.Transition.StateAfter))
        {
          canTransitionAllSubresources = false;
        }

        subresourceTransitions.Add(barrier);

        globalSubData.myStates = localSubData.myStates;
        if (localSubData.myWasWritten)
          globalSubData.myContext = aCommandList->GetType();
      }

      if (!subresourceTransitions.IsEmpty())
      {
        if (canTransitionAllSubresources)
        {
          D3D12_RESOURCE_BARRIER barrier = subresourceTransitions.GetFirst();
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          patchingBarriers.push_back(barrier);
        }
        else
        {
          patchingBarriers.reserve(patchingBarriers.size() + subresourceTransitions.Size());
          for (uint i = 0u; i < subresourceTransitions.Size(); ++i)
            patchingBarriers.push_back(subresourceTransitions[i]);
        }

        bool allSubresourcesSameState = true;
        D3D12_RESOURCE_STATES firstState = (D3D12_RESOURCE_STATES)globalHazardData.myDx12Data.mySubresources[0].myStates;
        CommandListType firstContext = globalHazardData.myDx12Data.mySubresources[0].myContext;
        for (uint subIdx = 1u; allSubresourcesSameState && subIdx < globalHazardData.myDx12Data.mySubresources.size(); ++subIdx)
          allSubresourcesSameState &=
          (firstState == globalHazardData.myDx12Data.mySubresources[subIdx].myStates &&
            firstContext == globalHazardData.myDx12Data.mySubresources[subIdx].myContext);

        globalHazardData.myDx12Data.myAllSubresourcesSameStates = allSubresourcesSameState;
      }
    }

    if (!patchingBarriers.empty())
    {
      CommandList* ctx = RenderCore::BeginCommandList(myType);
      CommandListDX12* ctxDx12 = static_cast<CommandListDX12*>(ctx);

      for (uint i = 0u; i < patchingBarriers.size(); ++i)
        ctxDx12->AddBarrier(patchingBarriers[i]);

      ctxDx12->FlushBarriers();

      ExecuteAndFreeCommandList(ctx);
    }
  }
//---------------------------------------------------------------------------//
}

#endif