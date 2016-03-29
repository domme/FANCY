#include "FancyCorePrerequisites.h"
#include "CommandAllocatorPoolDX12.h"
#include "Renderer.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//  
  CommandAllocatorPoolDX12::CommandAllocatorPoolDX12(Renderer& aRenderer, D3D12_COMMAND_LIST_TYPE aType)
    : myRenderer(aRenderer)
    , myCommandListType(aType)
  {
    
  }
//---------------------------------------------------------------------------//
  CommandAllocatorPoolDX12::~CommandAllocatorPoolDX12()
  {
    for (ID3D12CommandAllocator* allocator : myAvailableAllocators)
      allocator->Release();

    myAvailableAllocators.clear();

    for (auto allocatorEntry : myReleasedWaitingAllocators)
    {
      myRenderer.WaitForFence(allocatorEntry.first);
      allocatorEntry.second->Release();
    }
    myReleasedWaitingAllocators.clear();
  }
//---------------------------------------------------------------------------//
  ID3D12CommandAllocator* CommandAllocatorPoolDX12::GetNewAllocator()
  {
    // Check if some of the waiting allocators can be made available again
    while(!myReleasedWaitingAllocators.empty() && myRenderer.IsFenceDone(myReleasedWaitingAllocators.front().first))
    {
      myAvailableAllocators.push_back(myReleasedWaitingAllocators.front().second);
      myReleasedWaitingAllocators.pop_front();
    }

    if (!myAvailableAllocators.empty())
    {
      ID3D12CommandAllocator* allocator = myAvailableAllocators.front();
      myAvailableAllocators.pop_front();

      return allocator;
    }

    ID3D12CommandAllocator* allocator;
    CheckD3Dcall(myRenderer.GetDevice()->CreateCommandAllocator(myCommandListType, IID_PPV_ARGS(&allocator)));

    return allocator;
  }
//---------------------------------------------------------------------------//
  void CommandAllocatorPoolDX12::ReleaseAllocator(ID3D12CommandAllocator* anAllocator, uint64 anAllocatorDoneFenceVal)
  {
#if defined (FANCY_RENDERSYSTEM_USE_VALIDATION)
    for (ID3D12CommandAllocator* allocator : myAvailableAllocators)
      ASSERT(allocator != anAllocator);

    for (auto allocatorEntry : myReleasedWaitingAllocators)
      ASSERT(allocatorEntry.second != anAllocator);
#endif // FANCY_RENDERSYSTEM_USE_VALIDATION

    myReleasedWaitingAllocators.push_back(std::make_pair(anAllocatorDoneFenceVal, anAllocator));
  }
//---------------------------------------------------------------------------//
} } }