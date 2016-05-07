#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include <deque>

namespace Fancy{ namespace Rendering{
class RenderOutput;
}}

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class CommandAllocatorPoolDX12
  {
  public:
    CommandAllocatorPoolDX12(D3D12_COMMAND_LIST_TYPE aType);
    ~CommandAllocatorPoolDX12();

    ID3D12CommandAllocator* GetNewAllocator();
    void ReleaseAllocator(ID3D12CommandAllocator* anAllocator, uint64 anAllocatorDoneFenceVal);

  private:
    D3D12_COMMAND_LIST_TYPE myCommandListType;

    std::deque<ID3D12CommandAllocator*> myAvailableAllocators;
    std::deque<std::pair<uint64, ID3D12CommandAllocator*>> myReleasedWaitingAllocators;
  };
//---------------------------------------------------------------------------//  
} } }