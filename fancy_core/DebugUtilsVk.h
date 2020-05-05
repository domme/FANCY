#pragma once

#include "VkPrerequisites.h"

namespace Fancy
{
  namespace DebugUtilsVk
  {
    String AccessMaskToString(VkAccessFlags anAccessMask);
    String PipelineStageMaskToString(VkPipelineStageFlags aPipelineStageMask);
    String ImageLayoutToString(VkImageLayout anImageLayout);
  }
}

