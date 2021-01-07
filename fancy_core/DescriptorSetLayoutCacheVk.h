#pragma once

#include "VkPrerequisites.h"
#include "RenderPlatformObjectCache.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class DescriptorSetLayoutCacheVk : public RenderPlatformObjectCache<VkDescriptorSetLayout>
  {
  public:
    static uint64 GetHash(const eastl::span<const VkDescriptorSetLayoutBinding>& someBindings);

    DescriptorSetLayoutCacheVk() = default;
    ~DescriptorSetLayoutCacheVk();

    void Clear() override;

    VkDescriptorSetLayout GetDescriptorSetLayout(const eastl::span<const VkDescriptorSetLayoutBinding>& someBindings);
    VkDescriptorSetLayout TryGetDescriptorSetLayout(uint64 aHash);
  };

}

#endif