#include "fancy_core_precompile.h"
#include "FrameBufferCacheVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "GpuResourceViewDataVk.h"
#include "Texture.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  FrameBufferCacheVk::~FrameBufferCacheVk()
  {
    FrameBufferCacheVk::Clear();
  }
//---------------------------------------------------------------------------//
  VkFramebuffer FrameBufferCacheVk::GetCreate(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget, glm::uvec2 aFramebufferRes, VkRenderPass aRenderPass)
  {
    VkImageView attachments[RenderConstants::kMaxNumRenderTargets + 1u];
    for (uint i = 0u; i < aNumRenderTargets; ++i)
    {
      const GpuResourceViewDataVk& viewDataVk = someRendertargets[i]->myNativeData.To<GpuResourceViewDataVk>();
      attachments[i] = viewDataVk.myView.myImage;
    }

    const bool hasDepthStencilTarget = aDepthStencilTarget != nullptr;
    if (hasDepthStencilTarget)
    {
      const GpuResourceViewDataVk& viewDataVk = someRendertargets[aNumRenderTargets]->myNativeData.To<GpuResourceViewDataVk>();
      attachments[aNumRenderTargets] = viewDataVk.myView.myImage;
    }

    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = nullptr;
    framebufferInfo.flags = 0u;
    framebufferInfo.renderPass = aRenderPass;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.attachmentCount = hasDepthStencilTarget ? aNumRenderTargets + 1u : aNumRenderTargets;
    framebufferInfo.width = aFramebufferRes.x;
    framebufferInfo.height = aFramebufferRes.y;
    framebufferInfo.layers = 1u;  // TODO: Support layered rendering

    MathUtil::BeginMultiHash();
    MathUtil::AddToMultiHash(attachments, sizeof(VkImageView) * (hasDepthStencilTarget ? aNumRenderTargets + 1 : aNumRenderTargets));
    MathUtil::AddToMultiHash(&aRenderPass, sizeof(aRenderPass));
    MathUtil::AddToMultiHash(&aFramebufferRes, sizeof(aFramebufferRes));
    const uint64 hash = MathUtil::EndMultiHash();

    VkFramebuffer framebuffer = nullptr;
    ASSERT_VK_RESULT(vkCreateFramebuffer(RenderCore::GetPlatformVk()->myDevice, &framebufferInfo, nullptr, &framebuffer));

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myCache.find(hash);
    if (it != myCache.end())
    {
      vkDestroyFramebuffer(RenderCore::GetPlatformVk()->GetDevice(), framebuffer, nullptr);
      return it->second;
    }
  
    myCache[hash] = framebuffer;
    return framebuffer;
  }
//---------------------------------------------------------------------------//
  void FrameBufferCacheVk::Clear()
  {
    std::lock_guard<std::mutex> lock(myCacheMutex);

    RenderCore::WaitForIdle(CommandListType::Graphics);
    for (auto it = myCache.begin(); it != myCache.end(); ++it)
      vkDestroyFramebuffer(RenderCore::GetPlatformVk()->GetDevice(), it->second, nullptr);

    myCache.clear();
  }
//---------------------------------------------------------------------------//
}

#endif