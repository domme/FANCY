#pragma once

#include "FancyCorePrerequisites.h"
#include <deque>
#include "CommandListType.h"
#include "DX12Prerequisites.h"

namespace Fancy { namespace Rendering {
  class RenderOutput;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
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

    std::deque<ID3D12CommandAllocator*> myAvailableAllocators;
    std::deque<std::pair<uint64, ID3D12CommandAllocator*>> myReleasedWaitingAllocators;
  };  
//---------------------------------------------------------------------------//  
} } }