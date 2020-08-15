#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "RenderEnums.h"

#include "EASTL/fixed_vector.h"
#include "EASTL/fixed_list.h"

#if FANCY_ENABLE_DX12

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
    void UpdateAvailableAllocators(ID3D12CommandAllocator** aRequestedAllocator = nullptr);

    CommandListType myCommandListType;

    eastl::fixed_vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, 16> myAllocatorPool;
    eastl::fixed_list<ID3D12CommandAllocator*, 16> myAvailableAllocators;
    eastl::fixed_list<eastl::pair<uint64, ID3D12CommandAllocator*>, 16> myReleasedWaitingAllocators;
  };  
//---------------------------------------------------------------------------//  
}

#endif