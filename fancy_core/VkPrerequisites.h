#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.h"

namespace Fancy
{
  enum VkDebugConsts
  {
    VK_ASSERT_MISSING_IMPLEMENTATION = 0
  };

  namespace
  {
    void ASSERT_VK_RESULT(VkResult aResult)
    {
      if (aResult != VkResult::VK_SUCCESS)
        throw;
    }
  }
  
}
  

