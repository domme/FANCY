#include "fancy_core_precompile.h"
#include "PipelineLayoutCacheVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"
#include "EASTL/fixed_vector.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  PipelineLayoutCacheVk::~PipelineLayoutCacheVk()
  {
    PipelineLayoutCacheVk::Clear();
  }

  SharedPtr<PipelineLayoutVk> PipelineLayoutCacheVk::GetPipelineLayout(const PipelineLayoutCreateInfoVk& aCreateInfo)
  {
    eastl::fixed_vector<uint64, 32> setLayoutHashes;
    uint64 pipelineLayoutHash = 0ull;

    for (const PipelineLayoutCreateInfoVk::DescriptorSetInfo& setInfo : aCreateInfo.myDescriptorSetInfos)
    {
      eastl::span<const VkDescriptorSetLayoutBinding> bindings(setInfo.myBindings.begin(), setInfo.myBindings.end());
      const uint64 hash = DescriptorSetLayoutCacheVk::GetHash(bindings);
      setLayoutHashes.push_back(hash);
      MathUtil::hash_combine(pipelineLayoutHash, hash);
    }

    {
      std::lock_guard<std::mutex> lock(myCacheMutex);
      auto it = myCache.find(pipelineLayoutHash);
      if (it != myCache.end())
      {
        return it->second;
      }
    }

    // We don't have this pipeline layout cached yet. It needs to be created.
    // First, gather all descriptorSetLayouts, which could partially be already cached
    
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    DescriptorSetLayoutCacheVk& setLayoutCache = platformVk->GetDescriptorSetLayoutCache();

    eastl::fixed_vector<VkDescriptorSetLayout, 32> descSetLayouts;
    for (uint i = 0u; i < (uint) setLayoutHashes.size(); ++i)
    {
      VkDescriptorSetLayout layout = setLayoutCache.TryGetDescriptorSetLayout(setLayoutHashes[i]);
      if (layout == nullptr)
      {
        eastl::span<const VkDescriptorSetLayoutBinding> bindings(aCreateInfo.myDescriptorSetInfos[i].myBindings.begin(), aCreateInfo.myDescriptorSetInfos[i].myBindings.end());
        layout = setLayoutCache.GetDescriptorSetLayout(bindings);
      }
      
      ASSERT(layout != nullptr);
      descSetLayouts.push_back(layout);
    }

    // Create the actual pipeline layout
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.flags = 0u;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.pPushConstantRanges = nullptr;  // Don't use push constants for now
    layoutCreateInfo.pushConstantRangeCount = 0u;
    layoutCreateInfo.setLayoutCount = (uint)descSetLayouts.size();
    layoutCreateInfo.pSetLayouts = descSetLayouts.data();

    VkPipelineLayout pipelineLayout = nullptr;
    ASSERT_VK_RESULT(vkCreatePipelineLayout(platformVk->myDevice, &layoutCreateInfo, nullptr, &pipelineLayout));

    SharedPtr<PipelineLayoutVk> layout(new PipelineLayoutVk(pipelineLayout, aCreateInfo, descSetLayouts));

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myCache.find(pipelineLayoutHash);
    if (it != myCache.end())
    {
      vkDestroyPipelineLayout(platformVk->myDevice, pipelineLayout, nullptr);
      return it->second;
    }

    myCache[pipelineLayoutHash] = layout;
    return layout;
  }
//---------------------------------------------------------------------------//
  void PipelineLayoutCacheVk::Clear()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    std::lock_guard<std::mutex> lock(myCacheMutex);

    myCache.clear();
  }
//---------------------------------------------------------------------------//
}

#endif