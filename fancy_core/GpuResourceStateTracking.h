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
  struct GpuSubresourceStateTracking
  {
    GpuResourceUsageState myState = GpuResourceUsageState::COMMON;
    CommandListType myQueueType = CommandListType::Graphics;
  };
  //---------------------------------------------------------------------------//
  struct GpuResourceStateTracking
  {
    bool myCanChangeStates = true;
    bool myAllSubresourcesInSameStateAndContext = true;
    DynamicArray<GpuSubresourceStateTracking> mySubresources;
    GpuResourceStateTrackingDX12 myDx12Data;  // Will become a union once other platforms are in
  };
//---------------------------------------------------------------------------//

}


