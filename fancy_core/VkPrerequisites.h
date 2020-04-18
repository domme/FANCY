#pragma once

#if FANCY_ENABLE_VK

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan.h"

#include "Log.h"  // Only for ReportMissingVkImplementation(). Remove this dependency when Vulkan is feature-complete

namespace Fancy
{
  enum
  {
    kVkMaxNumBoundDescriptorSets = 32,
    kVkMaxNumDescriptorRangesPerSet = 64,
    kVkMaxNumDescriptorsPerRange = 64
  };

  enum VkImplementationDebugLevel
  {
    VK_IMPLEMENTATION_DEBUG_LEVEL_NONE = 0,
    VK_IMPLEMENTATION_DEBUG_LEVEL_LOG,
    VK_IMPLEMENTATION_DEBUG_LEVEL_ASSERT
  };

  enum VkDebugConsts
  {
    kVkImplementationDebugLevel = VK_IMPLEMENTATION_DEBUG_LEVEL_NONE
  };

  inline void ASSERT_VK_RESULT(VkResult aResult)
  {
    if (aResult != VkResult::VK_SUCCESS)
      throw;
  }

  inline void ReportMissingVkImplementation(const char* aFunction)
  {
    if (kVkImplementationDebugLevel > VK_IMPLEMENTATION_DEBUG_LEVEL_NONE)
      Log("Missing Vulkan Implementation: %s", aFunction);

    if ((uint) kVkImplementationDebugLevel == (uint) VK_IMPLEMENTATION_DEBUG_LEVEL_ASSERT)
      throw;
  }

#define VK_MISSING_IMPLEMENTATION(...) ReportMissingVkImplementation(__FUNCTION__)
}
  
#endif