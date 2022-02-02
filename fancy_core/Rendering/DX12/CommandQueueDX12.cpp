#include "fancy_core_precompile.h"
#include "CommandQueueDX12.h"

#include "Rendering/RenderCore.h"
#include "Debug/Log.h"

#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"
#include "CommandListDX12.h"
#include "GpuResourceDataDX12.h"
#include "DebugUtilsDX12.h"

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
	  ASSERT_HRESULT(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&myQueue)));

    ASSERT_HRESULT(device->CreateFence(myLastCompletedFenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&myFence)));
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
    ASSERT_HRESULT(myQueue->Signal(myFence.Get(), myNextFenceVal));
    return myNextFenceVal++;
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForFence(uint64 aFenceVal)
  {
    ASSERT(GetCommandListType(aFenceVal) == myType, "Can't compare against fence-value from different timeline");

    if (IsFenceDone(aFenceVal))
      return;

    ASSERT_HRESULT(myFence->SetEventOnCompletion(aFenceVal, myFenceCompletedEvent));
    WaitForSingleObject(myFenceCompletedEvent, INFINITE);
    myLastCompletedFenceVal = glm::max(myLastCompletedFenceVal, aFenceVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::WaitForIdle()
  {
    const uint64 fenceVal = SignalAndIncrementFence();
    WaitForFence(fenceVal);
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::StallForQueue(const CommandQueue* aCommandQueue)
  {
    const CommandQueueDX12* otherQueue = static_cast<const CommandQueueDX12*>(aCommandQueue);
    if (otherQueue != this)
      ASSERT_HRESULT(myQueue->Wait(otherQueue->myFence.Get(), otherQueue->myNextFenceVal - 1u));
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::StallForFence(uint64 aFenceVal)
  {
    const CommandQueueDX12* otherQueue = static_cast<const CommandQueueDX12*>(RenderCore::GetCommandQueue(aFenceVal));
    if (otherQueue != this)
      ASSERT_HRESULT(myQueue->Wait(otherQueue->myFence.Get(), aFenceVal));
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

    if (aSyncMode == SyncMode::BLOCKING || RenderCore::ourDebugWaitAfterEachSubmit)
      WaitForFence(fenceVal);

    return fenceVal;
  }
//---------------------------------------------------------------------------//
  uint64 CommandQueueDX12::ExecuteAndResetCommandListInternal(CommandList* aContext, SyncMode aSyncMode)
  {
    const uint64 fenceVal = ExecuteCommandListInternal(aContext, aSyncMode);
    aContext->ResetAndOpen();
    return fenceVal;
  }
//---------------------------------------------------------------------------//
  void CommandQueueDX12::ResolveResourceHazardData(CommandList* aCommandList)
  {
    eastl::fixed_vector<D3D12_RESOURCE_BARRIER, 64> patchingBarriers;

    CommandListDX12* cmdListDx12 = static_cast<CommandListDX12*>(aCommandList);

    for (auto it : cmdListDx12->myLocalHazardData)
    {
      const GpuResource* resource = it.first;

      GpuResourceHazardDataDX12& globalHazardData = resource->GetDX12Data()->myHazardData;
      const CommandListDX12::LocalHazardData& localHazardData = it.second;
      ASSERT(globalHazardData.mySubresources.size() == localHazardData.mySubresources.size());

      eastl::fixed_vector<D3D12_RESOURCE_BARRIER, 64> subresourceTransitions;
      for (uint subIdx = 0u; subIdx < (uint) localHazardData.mySubresources.size(); ++subIdx)
      {
        GpuSubresourceHazardDataDX12& globalSubData = globalHazardData.mySubresources[subIdx];
        const CommandListDX12::SubresourceHazardData& localSubData = localHazardData.mySubresources[subIdx];

        if (!localSubData.myWasUsed)
          continue;

        const D3D12_RESOURCE_STATES oldGlobalStates = globalSubData.myStates;

        if (localSubData.myIsSharedReadState)
        {
          ASSERT(!localSubData.myWasWritten);
          globalSubData.myContext = CommandListType::SHARED_READ;
        }

        globalSubData.myStates = localSubData.myStates;
        if (localSubData.myWasWritten)
          globalSubData.myContext = aCommandList->GetType();

        if (oldGlobalStates == localSubData.myFirstDstStates)
        {
#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
          if (RenderCore::ourDebugLogResourceBarriers)
            LOG_DEBUG("Patching subresource transition: %s (subresource %d): No transition needed (global state %s == first dst state on commandlist %s)", resource->GetName(), subIdx,
              DebugUtilsDX12::ResourceStatesToString(oldGlobalStates).c_str(), DebugUtilsDX12::ResourceStatesToString(localSubData.myFirstDstStates).c_str());
#endif
          continue;
        }

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.StateBefore = oldGlobalStates;
        barrier.Transition.StateAfter = localSubData.myFirstDstStates;
        barrier.Transition.Subresource = subIdx;
        barrier.Transition.pResource = eastl::any_cast<const GpuResourceDataDX12&>(resource->myNativeData).myResource.Get();

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        if (RenderCore::ourDebugLogResourceBarriers)
          LOG_DEBUG("Patching subresource transition: %s (subresource %d): %s -> %s", resource->GetName(), subIdx,
            DebugUtilsDX12::ResourceStatesToString(barrier.Transition.StateBefore).c_str(), DebugUtilsDX12::ResourceStatesToString(barrier.Transition.StateAfter).c_str());
#endif
        
        subresourceTransitions.push_back(barrier);
      }

      if (!subresourceTransitions.empty())
      {
        bool canTransitionAllSubresources = (uint) subresourceTransitions.size() == globalHazardData.mySubresources.size();
        if (canTransitionAllSubresources)
        {
          const D3D12_RESOURCE_BARRIER& firstBarrier = subresourceTransitions.front();
          for (uint i = 1u; canTransitionAllSubresources && i < (uint) subresourceTransitions.size(); ++i)
          {
            canTransitionAllSubresources &= firstBarrier.Transition.StateBefore == subresourceTransitions[i].Transition.StateBefore &&
              firstBarrier.Transition.StateAfter == subresourceTransitions[i].Transition.StateAfter;
          }
        }

        if (canTransitionAllSubresources)
        {
          D3D12_RESOURCE_BARRIER barrier = subresourceTransitions.front();
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          patchingBarriers.push_back(barrier);
        }
        else
        {
          patchingBarriers.reserve(patchingBarriers.size() + subresourceTransitions.size());
          for (uint i = 0u; i < subresourceTransitions.size(); ++i)
            patchingBarriers.push_back(subresourceTransitions[i]);
        }

        bool allSubresourcesSameState = true;
        D3D12_RESOURCE_STATES firstState = globalHazardData.mySubresources[0].myStates;
        CommandListType firstContext = globalHazardData.mySubresources[0].myContext;
        for (uint subIdx = 1u; allSubresourcesSameState && subIdx < globalHazardData.mySubresources.size(); ++subIdx)
          allSubresourcesSameState &=
          (firstState == globalHazardData.mySubresources[subIdx].myStates &&
            firstContext == globalHazardData.mySubresources[subIdx].myContext);

        globalHazardData.myAllSubresourcesSameStates = allSubresourcesSameState;
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