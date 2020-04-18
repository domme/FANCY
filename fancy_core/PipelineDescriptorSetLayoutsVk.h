#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct PipelineDescriptorSetLayoutsVk
  {
    const VkDescriptorSetLayout& operator[](uint anIndex) const { ASSERT(anIndex < ARRAY_LENGTH(myLayouts)); return myLayouts[anIndex]; }
    VkDescriptorSetLayout& operator[](uint anIndex) { ASSERT(anIndex < ARRAY_LENGTH(myLayouts)); return myLayouts[anIndex]; }
    VkDescriptorSetLayout myLayouts[kVkMaxNumBoundDescriptorSets] = {};
  };
}

#endif