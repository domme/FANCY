#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuResourceStateTrackingDX12
  {
    // Uints are D3D12_RESOURCE_STATES
    uint myReadStates;
    uint myWriteStates;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceStateTrackingVk
  {
    // Uints are VkAccessFlags
    uint myReadAccessMask;
    uint myWriteAccessMask;
    bool myHasExclusiveQueueAccess;
    mutable bool myHasInitialImageLayout;  // True if the image has never been used in a resourceBarrier and still has its initial layout from creation.
    uint myInitialImageLayout;  // Initial layout given to an image upon creation.
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardTracking
  {
    static bool QueueUnderstandsState(CommandListType aCurrQueue, CommandListType aQueue, GpuResourceState aState);
    static bool QueueUnderstandsPartsOfState(CommandListType aCurrQueue, GpuResourceState aState);
    static bool StateIsContainedIn(GpuResourceState aLowerState, GpuResourceState aHigherState);
    
    bool myCanChangeStates = true;
    GpuResourceState myDefaultState = GpuResourceState::READ_ANY_SHADER_ALL_BUT_DEPTH;

    union {
      GpuResourceStateTrackingDX12 myDx12Data;
      GpuResourceStateTrackingVk myVkData;
    };
  };
//---------------------------------------------------------------------------//
}
