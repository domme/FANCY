#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuResourceHazardTrackingDX12
  {
    // Uints are D3D12_RESOURCE_STATES
    uint myReadStates;
    uint myWriteStates;
    uint myState;
    CommandListType myContext;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardTrackingVk
  {
    // Uints are VkAccessFlags
    uint myReadAccessMask;
    uint myWriteAccessMask;
    bool myHasExclusiveQueueAccess;
    mutable bool myHasInitialImageLayout;  // True if the image has never been used in a resourceBarrier and still has its initial layout from creation.
    uint myInitialImageLayout;  // Initial layout given to an image upon creation.
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardData
  {
    bool myCanChangeStates = true;

    union {
      GpuResourceHazardTrackingDX12 myDx12Data;
      GpuResourceHazardTrackingVk myVkData;
    };
  };
//---------------------------------------------------------------------------//
}
