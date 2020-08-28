#pragma once

#include "VkPrerequisites.h"

namespace Fancy
{
  namespace DebugUtilsVk
  {
    eastl::string AccessMaskToString(VkAccessFlags anAccessMask);
    eastl::string PipelineStageMaskToString(VkPipelineStageFlags aPipelineStageMask);
    eastl::string ImageLayoutToString(VkImageLayout anImageLayout);
  }
}

