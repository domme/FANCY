#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

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
  };
}

#endif