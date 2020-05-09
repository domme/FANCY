#pragma once

#include "FancyCoreDefines.h"
#include "VkPrerequisites.h"
#include "RenderPlatformObjectCache.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct GraphicsPipelineState;
  struct ComputePipelineState;

  class PipelineStateCacheVk : public RenderPlatformObjectCache<VkPipeline>
  {
  public:
    ~PipelineStateCacheVk() override;

    VkPipeline GetCreateGraphicsPipeline(const GraphicsPipelineState& aState, VkRenderPass aRenderPass);
    VkPipeline GetCreateComputePipeline(const ComputePipelineState& aState);
    void Clear() override;
  };

}

#endif