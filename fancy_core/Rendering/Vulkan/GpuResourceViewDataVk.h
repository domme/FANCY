#pragma once

#if FANCY_ENABLE_VK

#include "Rendering/GlobalDescriptorAllocation.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  struct GpuResourceViewDataVk
  {
    enum Type
    {
      None = 0,
      Buffer,
      Image
    };

    union
    {
      VkImageView myImage;
      VkBufferView myBuffer;
    } myView;

    Type myType = None;
    GlobalDescriptorAllocation myGlobalDescriptor;
  };
}

#endif