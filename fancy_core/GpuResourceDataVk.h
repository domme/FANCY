#pragma once

#include "VkPrerequisites.h"
#include "RenderEnums.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
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
    VkAccessFlags myReadAccessMask;
    VkAccessFlags myWriteAccessMask;
    bool myHasExclusiveQueueAccess;
    uint mySupportedImageLayoutMask;
    DynamicArray<GpuSubresourceHazardDataVk> mySubresources;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceDataVk
  {
    GpuResourceHazardDataVk myHazardData;
    GpuResourceType myType;
    VkDeviceMemory myMemory;
    union {
      VkBuffer myBuffer;
      VkImage myImage;
    };
  };
}

#endif