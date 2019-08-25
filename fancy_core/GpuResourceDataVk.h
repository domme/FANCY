#pragma once

#include "VkPrerequisites.h"
#include "RenderEnums.h"

namespace Fancy
{
  struct GpuResourceDataVk
  {
    GpuResourceCategory myType;
    union
    {
      VkBuffer myBuffer;
      VkImage myImage;
    };
  };
}
