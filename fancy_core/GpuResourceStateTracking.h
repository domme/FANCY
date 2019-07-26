#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuResourceStateTrackingDX12
  {
    // Uints are D3D12_RESOURCE_STATES
    uint myReadStates = 0u;
    uint myWriteStates = 0u;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceStateTracking
  {
    static bool QueueUnderstandsState(CommandListType aCurrQueue, CommandListType aQueue, GpuResourceState aState);
    static bool QueueUnderstandsPartsOfState(CommandListType aCurrQueue, GpuResourceState aState);
    static bool StateIsContainedIn(GpuResourceState aLowerState, GpuResourceState aHigherState);
    
    bool myCanChangeStates = true;
    GpuResourceState myDefaultState = GpuResourceState::COMMON;
    GpuResourceStateTrackingDX12 myDx12Data;  // Will become a union once other platforms are in
  };
//---------------------------------------------------------------------------//
}
