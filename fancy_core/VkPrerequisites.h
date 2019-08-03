#pragma once

#include "vulkan/vulkan.h"

namespace {
  void ASSERT_VK_RESULT(VkResult aResult)
  {
    if (aResult != VkResult::VK_SUCCESS)
      throw;
  }
}

