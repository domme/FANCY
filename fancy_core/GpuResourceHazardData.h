#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuSubresourceHazardDataDX12
  {
    uint myStates;
    CommandListType myContext;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardDataDX12
  {
    // Uints are D3D12_RESOURCE_STATES
    uint myReadStates;
    uint myWriteStates;
    bool myAllSubresourcesSameStates;
    DynamicArray<GpuSubresourceHazardDataDX12> mySubresources;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardDataVk
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

    // union {
      GpuResourceHazardDataDX12 myDx12Data;
      GpuResourceHazardDataVk myVkData;
   // };
  };
//---------------------------------------------------------------------------//
}
