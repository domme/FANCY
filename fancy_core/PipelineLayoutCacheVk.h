#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct PipelineDescriptorSetLayoutsVk;

  class PipelineLayoutCacheVk
  {
  public:
    PipelineLayoutCacheVk() = default;
    ~PipelineLayoutCacheVk();

    struct DescriptorSetInfo
    {
      DescriptorSetInfo(uint aSetIndex) : mySet(aSetIndex) { }
      uint mySet;
      DynamicArray<VkDescriptorSetLayoutBinding> myBindings;
    };

    VkPipelineLayout GetPipelineLayout(const DynamicArray<DescriptorSetInfo>& someDescriptorSetInfos, PipelineDescriptorSetLayoutsVk& aDescriptorSetLayoutsOut);
   
  private:
    std::unordered_map<uint64, VkDescriptorSetLayout> myDescriptorSetLayouts;
    std::unordered_map<uint64, VkPipelineLayout> myPipelineLayouts;
  };
}

#endif