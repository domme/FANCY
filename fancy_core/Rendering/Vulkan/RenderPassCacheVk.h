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

    // aResolveSharedReadLayouts: use GENERAL for depth-stencil targets in the SHARED_READ context of the legacy hazard tracking. Lists without hazard
    // tracking (frame graph) pass false and get the plain layouts.
    VkRenderPass GetCreate(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget, bool aResolveSharedReadLayouts);
    void Clear() override;
  };

}

#endif