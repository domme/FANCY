#pragma once

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
  };
}
