#pragma once

#if FANCY_ENABLE_VK

#include "Common/FancyCoreDefines.h"

#include "Rendering/RenderPlatformObjectCache.h"

#include "VkPrerequisites.h"

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