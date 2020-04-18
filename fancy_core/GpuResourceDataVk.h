#pragma once

#include "VkPrerequisites.h"
#include "RenderEnums.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct GpuResourceDataVk
  {
    GpuResourceCategory myType;
    VkDeviceMemory myMemory;
    union
    {
      VkBuffer myBuffer;
      VkImage myImage;
    };
  };
}

#endif