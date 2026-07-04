#pragma once

#include "Common/FancyCoreDefines.h"
#include "Rendering/RenderPlatformObjectCache.h"

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class TextureView;

  class FrameBufferCacheVk : public RenderPlatformObjectCache<VkFramebuffer>
  {
  public:
    ~FrameBufferCacheVk() override;

    VkFramebuffer GetCreate(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget, glm::uvec2 aFramebufferRes, VkRenderPass aRenderPass);
    void Clear() override;
  };

  
}

#endif
