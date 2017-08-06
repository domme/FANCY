#include "FancyCorePrerequisites.h"
#include "CommandAllocatorPoolDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//  
  CommandAllocatorPoolDX12::CommandAllocatorPoolDX12(CommandListType aType)
    : myCommandListType(aType)
  {
    
  }
//---------------------------------------------------------------------------//
  CommandAllocatorPoolDX12::~CommandAllocatorPoolDX12()
  {
    for (ID3D12CommandAllocator* allocator : myAvailableAllocators)
      allocator->Release();

    myAvailableAllocators.clear();
    myReleasedWaitingAllocators.clear();
  }
//---------------------------------------------------------------------------//
  ID3D12CommandAllocator* CommandAllocatorPoolDX12::GetNewAllocator()
  {
    // Check if some of the waiting allocators can be made available again
    while(!myReleasedWaitingAllocators.empty() 
      && RenderCore::GetPlatformDX12()->IsFenceDone(myCommandListType, 
                                 myReleasedWaitingAllocators.front().first))
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

    D3D12_COMMAND_LIST_TYPE nativeCmdListType = RenderCore_PlatformDX12::GetCommandListType(myCommandListType);

    ID3D12CommandAllocator* allocator;
    CheckD3Dcall(RenderCore::GetPlatformDX12()->GetDevice()->CreateCommandAllocator(nativeCmdListType, IID_PPV_ARGS(&allocator)));

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
