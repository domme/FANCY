#include "fancy_core_precompile.h"
#include "RenderPassCacheVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "Texture.h"
#include "GpuResourceDataVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Private
  {
    VkImageLayout ResolveImageLayout(VkImageLayout aLayout, const GpuResource* aResource, const SubresourceRange& aSubresourceRange)
    {
      const GpuResourceHazardDataVk& globalHazardData = aResource->GetVkData()->myHazardData;
      for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it)
      {
        const uint subresourceIdx = aResource->GetSubresourceIndex(*it);
        if (globalHazardData.mySubresources[subresourceIdx].myContext == CommandListType::SHARED_READ)
          return VK_IMAGE_LAYOUT_GENERAL;
      }

      return aLayout;
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  RenderPassCacheVk::~RenderPassCacheVk()
  {
    RenderPassCacheVk::Clear();
  }
//---------------------------------------------------------------------------//
  VkRenderPass RenderPassCacheVk::GetCreate(const TextureView** someRendertargets, uint aNumRenderTargets, const TextureView* aDepthStencilTarget)
  {
    DataFormat rtFormats[RenderConstants::kMaxNumRenderTargets];
    for (uint i = 0u; i < aNumRenderTargets; ++i)
    {
      const TextureView* renderTarget = someRendertargets[i];
      ASSERT(renderTarget != nullptr);
      rtFormats[i] = renderTarget->GetProperties().myFormat;
    }

    MathUtil::BeginMultiHash();
    MathUtil::AddToMultiHash(rtFormats, sizeof(DataFormat) * aNumRenderTargets);
    if (aDepthStencilTarget != nullptr)
    {
      const TextureViewProperties& dsvProps = aDepthStencilTarget->GetProperties();
      MathUtil::AddToMultiHash(dsvProps.myFormat);
      // Read-only DSVs need to produce a different renderpass-hash 
      MathUtil::AddToMultiHash(dsvProps.myIsDepthReadOnly ? 1u : 0u);
      MathUtil::AddToMultiHash(dsvProps.myIsStencilReadOnly ? 1u : 0u);
    }
    const uint64 hash = MathUtil::EndMultiHash();

    {
      std::lock_guard<std::mutex> lock(myCacheMutex);
      auto it = myCache.find(hash);
      if (it != myCache.end())
        return it->second;
    }

    VkRenderPassCreateInfo renderpassInfo;
    renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpassInfo.pNext = nullptr;
    renderpassInfo.flags = 0u;

    VkAttachmentDescription attachmentDescriptions[RenderConstants::kMaxNumRenderTargets + 1u];
    VkAttachmentReference colorAttachmentRefs[RenderConstants::kMaxNumRenderTargets];
    VkAttachmentReference depthStencilAttachmentRef;
    for (uint i = 0u; i < aNumRenderTargets; ++i)
    {
      VkAttachmentDescription& attachmentDesc = attachmentDescriptions[i];
      attachmentDesc.flags = 0u;
      attachmentDesc.format = RenderCore_PlatformVk::ResolveFormat(someRendertargets[i]->GetProperties().myFormat);
      attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
      const VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      attachmentDesc.initialLayout = layout;
      attachmentDesc.finalLayout = layout;
      attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

      VkAttachmentReference& attachmentRef = colorAttachmentRefs[i];
      attachmentRef.attachment = i;
      attachmentRef.layout = layout;
    }

    const bool hasDepthStencilTarget = aDepthStencilTarget != nullptr;
    if (hasDepthStencilTarget)
    {
      const TextureViewProperties& dsvProps = aDepthStencilTarget->GetProperties();

      VkAttachmentDescription& attachmentDesc = attachmentDescriptions[aNumRenderTargets];
      attachmentDesc.flags = 0u;
      attachmentDesc.format = RenderCore_PlatformVk::ResolveFormat(dsvProps.myFormat);
      attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

      VkImageLayout layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      if (dsvProps.myIsDepthReadOnly && dsvProps.myIsStencilReadOnly)
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
      else if (dsvProps.myIsDepthReadOnly)
        layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
      else if (dsvProps.myIsStencilReadOnly)
        layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

      layout = Private::ResolveImageLayout(layout, aDepthStencilTarget->GetResource(), aDepthStencilTarget->GetSubresourceRange());

      attachmentDesc.initialLayout = layout;
      attachmentDesc.finalLayout = layout;
      attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

      depthStencilAttachmentRef.attachment = aNumRenderTargets;
      depthStencilAttachmentRef.layout = layout;
    }
    renderpassInfo.attachmentCount = aNumRenderTargets + (aDepthStencilTarget != nullptr ? 1u : 0u);
    renderpassInfo.pAttachments = attachmentDescriptions;

    VkSubpassDescription subpassDesc;
    subpassDesc.flags = 0u;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = aNumRenderTargets;
    subpassDesc.pColorAttachments = colorAttachmentRefs;
    subpassDesc.pDepthStencilAttachment = aDepthStencilTarget != nullptr ? &depthStencilAttachmentRef : nullptr;
    subpassDesc.pResolveAttachments = nullptr;

    // Input attachments are a way to read the current value of a pixel that's currently being written in a fragment shader.
    // This would enable some advanced effects like transparency and things like that without having to use multiple passes.
    // For now, we'll disable that here, but will look into supporting this feature in the future if it has a corresponding feature in DX12
    subpassDesc.inputAttachmentCount = 0u;
    subpassDesc.pInputAttachments = nullptr;

    subpassDesc.preserveAttachmentCount = 0u;
    subpassDesc.pPreserveAttachments = nullptr;

    renderpassInfo.pSubpasses = &subpassDesc;
    renderpassInfo.subpassCount = 1u;
    renderpassInfo.pDependencies = nullptr;
    renderpassInfo.dependencyCount = 0u;

    VkRenderPass renderPass;
    ASSERT_VK_RESULT(vkCreateRenderPass(RenderCore::GetPlatformVk()->myDevice, &renderpassInfo, nullptr, &renderPass));

    std::lock_guard<std::mutex> lock(myCacheMutex);

    auto it = myCache.find(hash);
    if (it != myCache.end())
    {
      vkDestroyRenderPass(RenderCore::GetPlatformVk()->GetDevice(), renderPass, nullptr);
      return it->second;
    }

    myCache[hash] = renderPass;
    return renderPass;
  }
//---------------------------------------------------------------------------//
  void RenderPassCacheVk::Clear()
  {
    std::lock_guard<std::mutex> lock(myCacheMutex);

    RenderCore::WaitForIdle(CommandListType::Graphics);
    for (auto it = myCache.begin(); it != myCache.end(); ++it)
      vkDestroyRenderPass(RenderCore::GetPlatformVk()->GetDevice(), it->second, nullptr);

    myCache.clear();
  }
//---------------------------------------------------------------------------//
}

#endif