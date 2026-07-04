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
    GpuResourceType myType = GpuResourceType::BUFFER;
    VkDeviceMemory myMemory = nullptr;

    struct Buffer
    {
      VkBuffer myBuffer = nullptr;
      VkDeviceAddress myAddress = 0ull;
      VkAccelerationStructureKHR myAccelerationStructure = nullptr;
      VkDeviceAddress myAccelerationStructureAddress = 0ull;
    };
    Buffer myBufferData;
    VkImage myImage = nullptr;
  };
}

#endif