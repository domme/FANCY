#pragma once

#include "VkPrerequisites.h"
#include "RenderPlatformObjectCache.h"
#include "PipelineLayoutVk.h"
#include "EASTL/hash_map.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class PipelineLayoutCacheVk : public RenderPlatformObjectCache<SharedPtr<PipelineLayoutVk>>
  {
  public:
    ~PipelineLayoutCacheVk() override;

    SharedPtr<PipelineLayoutVk> GetPipelineLayout(const PipelineLayoutCreateInfoVk& aCreateInfo);

    void Clear() override;
  };
}

#endif