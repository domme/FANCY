#pragma once

#include "VkPrerequisites.h"
#include "RenderPlatformObjectCache.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct PipelineDescriptorSetLayoutsVk;

  class PipelineLayoutCacheVk : public RenderPlatformObjectCache<VkPipelineLayout>
  {
  public:
    ~PipelineLayoutCacheVk() override;

    struct DescriptorSetInfo
    {
      DescriptorSetInfo(uint aSetIndex) : mySet(aSetIndex) { }
      uint mySet;
      DynamicArray<VkDescriptorSetLayoutBinding> myBindings;
    };

    VkPipelineLayout GetPipelineLayout(const DynamicArray<DescriptorSetInfo>& someDescriptorSetInfos, PipelineDescriptorSetLayoutsVk& aDescriptorSetLayoutsOut);
    void Clear() override;
   
  private:
    std::unordered_map<uint64, VkDescriptorSetLayout> myDescriptorSetLayouts;
  };
}

#endif