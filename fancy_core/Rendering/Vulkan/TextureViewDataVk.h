#pragma once

#if FANCY_ENABLE_VK

#include "VkPrerequisites.h"

namespace Fancy
{
  struct TextureViewDataVk
  {
    VkImageView myImageView = nullptr;
  };
}

#endif
