#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.h"

#include "Log.h"  // Only for ReportMissingVkImplementation(). Remove this dependency when Vulkan is feature-complete

namespace Fancy
{
  enum VkDebugLevel
  {
    LOG_MISSING_IMPLEMENTATION = 0,
    ASSERT_MISSING_IMPLEMENTATION
  };

  enum VkDebugConsts
  {
    kVkDebugLevel = LOG_MISSING_IMPLEMENTATION
  };

  namespace
  {
    void ASSERT_VK_RESULT(VkResult aResult)
    {
      if (aResult != VkResult::VK_SUCCESS)
        throw;
    }

    void ReportMissingVkImplementation(const char* aFunction)
    {
      if (kVkDebugLevel == LOG_MISSING_IMPLEMENTATION)

    }

#define VK_MISSING_IMPLEMENTATION
  }
  
}
  

