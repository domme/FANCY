#pragma once

#include "FancyCoreDefines.h"

#if FANCY_ENABLE_VK

#include "VkPrerequisites.h"

namespace Fancy
{
  struct GlobalDescriptorAllocation
  {
    uint myIndex = UINT_MAX;
    GlobalResourceType myResourceType = GLOBAL_RESOURCE_NUM;
  };
}

#endif