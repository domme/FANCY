#include "fancy_core_precompile.h"
#include "PipelineLayoutCacheVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"
#include "PipelineDescriptorSetLayoutsVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  PipelineLayoutCacheVk::~PipelineLayoutCacheVk()
  {
    PipelineLayoutCacheVk::Clear();
  }

  VkPipelineLayout PipelineLayoutCacheVk::GetPipelineLayout(const DynamicArray<DescriptorSetInfo>& someDescriptorSetInfos, PipelineDescriptorSetLayoutsVk& aDescriptorSetLayoutsOut)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    eastl::fixed_vector<VkDescriptorSetLayout, 32> descSetLayouts;

    uint64 pipelineLayoutHash = 0ull;
    for (const DescriptorSetInfo& setInfo : someDescriptorSetInfos)
    {
      const uint descriptorSetIndex = setInfo.mySet;

      uint64 setLayoutHash = 0ull;
      for (const VkDescriptorSetLayoutBinding& bindingInSet : setInfo.myBindings)
      {
        MathUtil::hash_combine(setLayoutHash, bindingInSet.binding);
        MathUtil::hash_combine(setLayoutHash, (uint)bindingInSet.descriptorType);
        MathUtil::hash_combine(setLayoutHash, bindingInSet.descriptorCount);
        MathUtil::hash_combine(setLayoutHash, (uint)bindingInSet.stageFlags);
      }

      MathUtil::hash_combine(pipelineLayoutHash, setLayoutHash);

      std::lock_guard<std::mutex> lock(myCacheMutex);
      auto it = myDescriptorSetLayouts.find(setLayoutHash);
      if (it != myDescriptorSetLayouts.end())
      {
        descSetLayouts.push_back(it->second);
      }
      else
      {
        VkDescriptorSetLayoutCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0u;
        createInfo.bindingCount = (uint)setInfo.myBindings.size();
        createInfo.pBindings = setInfo.myBindings.data();

        VkDescriptorSetLayout setLayout = nullptr;
        ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(platformVk->myDevice, &createInfo, nullptr, &setLayout));
        myDescriptorSetLayouts[setLayoutHash] = setLayout;
        descSetLayouts.push_back(setLayout);
      }

      aDescriptorSetLayoutsOut[setInfo.mySet] = descSetLayouts.back();
    }

    {
      std::lock_guard<std::mutex> lock(myCacheMutex);
      auto it = myCache.find(pipelineLayoutHash);
      if (it != myCache.end())
      {
        return it->second;
      }
    }

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

    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myCache.find(pipelineLayoutHash);
    if (it != myCache.end())
    {
      vkDestroyPipelineLayout(platformVk->myDevice, pipelineLayout, nullptr);
      return it->second;
    }

    myCache[pipelineLayoutHash] = pipelineLayout;
    return pipelineLayout;
  }
//---------------------------------------------------------------------------//
  void PipelineLayoutCacheVk::Clear()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    std::lock_guard<std::mutex> lock(myCacheMutex);

    for (auto layout : myCache)
      vkDestroyPipelineLayout(platformVk->myDevice, layout.second, nullptr);

    for (auto layout : myDescriptorSetLayouts)
      vkDestroyDescriptorSetLayout(platformVk->myDevice, layout.second, nullptr);
  }
//---------------------------------------------------------------------------//
}

#endif