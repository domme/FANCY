#pragma once

#include "FancyCorePrerequisites.h"
#include "CommandListType.h"
#include "DX12Prerequisites.h"

#include <list>

namespace Fancy { 
//---------------------------------------------------------------------------//
  class CommandAllocatorPoolDX12
  {
  public:
    CommandAllocatorPoolDX12(CommandListType aType);
    ~CommandAllocatorPoolDX12();

    ID3D12CommandAllocator* GetNewAllocator();
    void ReleaseAllocator(ID3D12CommandAllocator* anAllocator, uint64 anAllocatorDoneFenceVal);

  private:
    CommandListType myCommandListType;

    std::vector<UniquePtr<ID3D12CommandAllocator>> myAllocatorPool;
    std::list<ID3D12CommandAllocator*> myAvailableAllocators;
    std::list<std::pair<uint64, ID3D12CommandAllocator*>> myReleasedWaitingAllocators;
  };  
//---------------------------------------------------------------------------//  
}
