#pragma once

#if FANCY_ENABLE_VK

#include "Rendering/RenderEnums.h"

#include "VkPrerequisites.h"

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
    eastl::fixed_vector<GpuSubresourceHazardDataVk, 16> mySubresources;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceDataVk
  {
    GpuResourceHazardDataVk myHazardData;
    GpuResourceType myType;
    VkDeviceMemory myMemory;

    struct Buffer
    {
      VkBuffer myBuffer;
      VkDeviceAddress myAddress;
      VkAccelerationStructureKHR myAccelerationStructure;
      VkDeviceAddress myAccelerationStructureAddress;
    };

    union {
      Buffer myBufferData;
      VkImage myImage;
    };
  };
}

#endif