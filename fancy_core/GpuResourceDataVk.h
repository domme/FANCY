#pragma once

#include "VkPrerequisites.h"
#include "RenderEnums.h"

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
