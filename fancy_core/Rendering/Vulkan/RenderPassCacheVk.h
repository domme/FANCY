#pragma once

#if FANCY_ENABLE_VK

#include "VkPrerequisites.h"
#include "Common/FancyCoreDefines.h"
#include "Rendering/RenderPlatformObjectCache.h"

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