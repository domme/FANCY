#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy
{
  struct GlobalDescriptorAllocation
  {
    uint myIndex = UINT_MAX;
    GlobalResourceType myResourceType = GLOBAL_RESOURCE_NUM;
  };
}
