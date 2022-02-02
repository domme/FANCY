#pragma once

#if FANCY_ENABLE_VK

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.hpp"

namespace Fancy
{
  inline void ASSERT_VK_RESULT(VkResult aResult)
  {
    if (aResult != VkResult::VK_SUCCESS)
      throw;
  }
}
  
#endif