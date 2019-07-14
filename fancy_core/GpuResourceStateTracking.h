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
    static bool QueueUnderstandsState(CommandListType aCurrQueue, CommandListType aQueue, GpuResourceUsageState aState);
    static bool IsBarrierNeeded(CommandListType aSrcQueue, GpuResourceUsageState aSrcState, CommandListType aDstQueue, GpuResourceUsageState aDstState);
    static bool StateIsContainedIn(GpuResourceUsageState aLowerState, GpuResourceUsageState aHigherState);
    
    bool myCanChangeStates = true;
    GpuResourceUsageState myState = GpuResourceUsageState::COMMON;
    CommandListType myQueueType = CommandListType::Graphics;

    GpuResourceStateTrackingDX12 myDx12Data;  // Will become a union once other platforms are in
  };
//---------------------------------------------------------------------------//
}
