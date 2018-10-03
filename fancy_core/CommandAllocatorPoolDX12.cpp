#include "FancyCorePrerequisites.h"
#include "CommandAllocatorPoolDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//  
  CommandAllocatorPoolDX12::CommandAllocatorPoolDX12(CommandListType aType)
    : myCommandListType(aType)
  {
    
  }
//---------------------------------------------------------------------------//
  CommandAllocatorPoolDX12::~CommandAllocatorPoolDX12()
  {
  }
//---------------------------------------------------------------------------//
  ID3D12CommandAllocator* CommandAllocatorPoolDX12::GetNewAllocator()
  {
    // Check if some of the waiting allocators can be made available again
    ID3D12CommandAllocator* availableAllocator = nullptr;
    auto it = myReleasedWaitingAllocators.begin();
    while(it != myReleasedWaitingAllocators.end())
    {
      const uint64 waitingFenceVal = it->first;
      ID3D12CommandAllocator* allocator = it->second;
      CommandQueueDX12* queue = (CommandQueueDX12*) RenderCore::GetCommandQueue(myCommandListType);

      if (queue->IsFenceDone(waitingFenceVal))
      {
        if (waitingFenceVal != 0)
          allocator->Reset();
        it = myReleasedWaitingAllocators.erase(it);

        if (availableAllocator == nullptr)
          availableAllocator = allocator;
        else
          myAvailableAllocators.push_back(allocator);
      }
      else
        ++it;
    }

    if (availableAllocator)
      return availableAllocator;

    if (!myAvailableAllocators.empty())
    {
      ID3D12CommandAllocator* allocator = myAvailableAllocators.front();
      myAvailableAllocators.pop_front();

      return allocator;
    }

    D3D12_COMMAND_LIST_TYPE nativeCmdListType = RenderCore_PlatformDX12::GetCommandListType(myCommandListType);

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->CreateCommandAllocator(nativeCmdListType, IID_PPV_ARGS(&allocator)));
    myAllocatorPool.push_back(allocator);

    return myAllocatorPool.back().Get();
  }
//---------------------------------------------------------------------------//
  void CommandAllocatorPoolDX12::ReleaseAllocator(ID3D12CommandAllocator* anAllocator, uint64 anAllocatorDoneFenceVal)
  {
#if defined (FANCY_RENDERER_HEAVY_VALIDATION)
    for (ID3D12CommandAllocator* allocator : myAvailableAllocators)
      ASSERT(allocator != anAllocator);

    for (auto allocatorEntry : myReleasedWaitingAllocators)
      ASSERT(allocatorEntry.second != anAllocator);
#endif // FANCY_RENDERSYSTEM_USE_VALIDATION

    myReleasedWaitingAllocators.push_back(std::make_pair(anAllocatorDoneFenceVal, anAllocator));
  }
//---------------------------------------------------------------------------//
}
