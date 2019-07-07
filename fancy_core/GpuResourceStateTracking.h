#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
  struct GpuResourceTransitionInfo
  {
    bool myCanTransitionFromSrc = false;
    bool myCanTransitionToDst = false;
    bool myCanFullyTransitionFromSrc = false;
    bool myCanFullyTransitionToDst = false;
  };
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
    static bool QueueCanTransitionFrom(CommandListType aQueue, CommandListType aSrcQueue, GpuResourceUsageState aSrcState);
    static bool IsBarrierNeeded(CommandListType aSrcQueue, GpuResourceUsageState aSrcState, CommandListType aDstQueue, GpuResourceUsageState aDstState);
    static GpuResourceUsageState GetMoreGenericState(GpuResourceUsageState aState1, GpuResourceUsageState aState2);
    
    bool myCanChangeStates = true;
    GpuResourceUsageState myState = GpuResourceUsageState::COMMON;
    CommandListType myQueueType = CommandListType::Graphics;

    GpuResourceStateTrackingDX12 myDx12Data;  // Will become a union once other platforms are in
  };
//---------------------------------------------------------------------------//

}


