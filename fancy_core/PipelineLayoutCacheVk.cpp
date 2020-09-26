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
      uint64 setLayoutHash = 0ull;
      for (const VkDescriptorSetLayoutBinding& bindingInSet : setInfo.myBindings)
      {
        MathUtil::hash_combine(setLayoutHash, bindingInSet.binding);
        MathUtil::hash_combine(setLayoutHash, (uint)bindingInSet.descriptorType);
        MathUtil::hash_combine(setLayoutHash, bindingInSet.descriptorCount);
        MathUtil::hash_combine(setLayoutHash, (uint)bindingInSet.stageFlags);
      }

      setLayoutHashes.push_back(setLayoutHash);
      MathUtil::hash_combine(pipelineLayoutHash, setLayoutHash);
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

    eastl::fixed_vector<VkDescriptorSetLayout, 32> descSetLayouts;

    {
      std::lock_guard<std::mutex> lock(myCacheMutex);
      ASSERT(setLayoutHashes.size() == aCreateInfo.myDescriptorSetInfos.size());

      for (uint i = 0u; i < (uint) setLayoutHashes.size(); ++i)
      {
        auto it = myDescriptorSetLayouts.find(setLayoutHashes[i]);
        if (it != myDescriptorSetLayouts.end())
        {
          descSetLayouts.push_back(it->second);
        }
        else
        {
          const PipelineLayoutCreateInfoVk::DescriptorSetInfo& setInfo = aCreateInfo.myDescriptorSetInfos[i];

          eastl::fixed_vector<VkDescriptorBindingFlags, 16> bindingFlags;
          bindingFlags.resize(setInfo.myBindings.size());

          // Always allow for arrays to be partially bound
          for (uint iBinding = 0u; iBinding < (uint)setInfo.myBindings.size(); ++iBinding)
            bindingFlags[iBinding] = setInfo.myBindings[iBinding].descriptorCount > 1 ? VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT : 0u;

          VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
          extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
          extendedInfo.pNext = nullptr;
          extendedInfo.bindingCount = (uint) setInfo.myBindings.size();
          extendedInfo.pBindingFlags = bindingFlags.data();

          VkDescriptorSetLayoutCreateInfo createInfo;
          createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
          createInfo.pNext =  &extendedInfo;
          createInfo.flags = 0u;
          createInfo.bindingCount = (uint)setInfo.myBindings.size();
          createInfo.pBindings = setInfo.myBindings.data();
          
          VkDescriptorSetLayout setLayout = nullptr;

          if (createInfo.bindingCount > 0) // Could be 0 if not all descriptor set-indices are used (e.g. if 1 is the first used set in a shader, then set 0 will have no ranges at all)
            ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(platformVk->myDevice, &createInfo, nullptr, &setLayout));

          myDescriptorSetLayouts[setLayoutHashes[i]] = setLayout;
          descSetLayouts.push_back(setLayout);
        }
      }
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

    for (auto layout : myDescriptorSetLayouts)
      vkDestroyDescriptorSetLayout(platformVk->myDevice, layout.second, nullptr);

    myDescriptorSetLayouts.clear();
  }
//---------------------------------------------------------------------------//
}

#endif