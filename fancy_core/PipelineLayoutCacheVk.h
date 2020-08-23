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
      eastl::fixed_vector<VkDescriptorSetLayoutBinding, 16> myBindings;
    };

    VkPipelineLayout GetPipelineLayout(const eastl::fixed_vector<DescriptorSetInfo, 16>& someDescriptorSetInfos, PipelineDescriptorSetLayoutsVk& aDescriptorSetLayoutsOut);
    void Clear() override;
   
  private:
    std::unordered_map<uint64, VkDescriptorSetLayout> myDescriptorSetLayouts;
  };
}

#endif