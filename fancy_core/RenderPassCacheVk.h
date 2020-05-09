#pragma once

#include "FancyCoreDefines.h"
#include "VkPrerequisites.h"
#include "RenderPlatformObjectCache.h"


#if FANCY_ENABLE_VK

namespace Fancy
{
  class RenderPassCacheVk : public RenderPlatformObjectCache<VkRenderPass>
  {
  public:
    ~RenderPassCacheVk() override;

    VkRenderPass GetCreate(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget);
    void Clear() override;
  };

}

#endif