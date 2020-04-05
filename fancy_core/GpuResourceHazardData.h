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
  struct GpuSubresourceHazardDataVk
  {
    uint myImageLayout;
    uint myAccessMask;
    // uint myLastWrittenPipelineStageMask;  // TODO: Support this
    CommandListType myContext;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardDataVk
  {
    // Uints are VkAccessFlags
    uint myReadAccessMask;
    uint myWriteAccessMask;
    bool myHasExclusiveQueueAccess;
    DynamicArray<GpuSubresourceHazardDataVk> mySubresources;
    DynamicArray<uint> mySupportedImageLayouts;
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
