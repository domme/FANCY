#include "fancy_core_precompile.h"
#include "DescriptorSetLayoutCacheVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  uint64 DescriptorSetLayoutCacheVk::GetHash(const eastl::span<const VkDescriptorSetLayoutBinding>& someBindings)
  {
    uint64 setLayoutHash = 0ull;
    for (const VkDescriptorSetLayoutBinding& bindingInSet : someBindings)
    {
      MathUtil::hash_combine(setLayoutHash, bindingInSet.binding);
      MathUtil::hash_combine(setLayoutHash, (uint)bindingInSet.descriptorType);
      MathUtil::hash_combine(setLayoutHash, bindingInSet.descriptorCount);
      MathUtil::hash_combine(setLayoutHash, (uint)bindingInSet.stageFlags);
    }
    return setLayoutHash;
  }
//---------------------------------------------------------------------------//
  DescriptorSetLayoutCacheVk::~DescriptorSetLayoutCacheVk()
  {
    DescriptorSetLayoutCacheVk::Clear();
  }
//---------------------------------------------------------------------------//
  void DescriptorSetLayoutCacheVk::Clear()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    std::lock_guard<std::mutex> lock(myCacheMutex);

    for (eastl::pair<uint64, VkDescriptorSetLayout> cacheEntry : myCache)
      vkDestroyDescriptorSetLayout(platformVk->myDevice, cacheEntry.second, nullptr);

    myCache.clear();
  }
//---------------------------------------------------------------------------//
  VkDescriptorSetLayout DescriptorSetLayoutCacheVk::GetDescriptorSetLayout(const eastl::span<const VkDescriptorSetLayoutBinding>& someBindings)
  {
    const uint64 hash = GetHash(someBindings);

    std::lock_guard<std::mutex> lock(myCacheMutex);

    VkDescriptorSetLayout layout = TryGetDescriptorSetLayout(hash);
    if (layout != nullptr)
      return layout;

    eastl::fixed_vector<VkDescriptorBindingFlags, 16> bindingFlags;
    bindingFlags.resize(someBindings.size());

    // Always allow for arrays to be partially bound
    for (uint iBinding = 0u; iBinding < (uint)someBindings.size(); ++iBinding)
      bindingFlags[iBinding] = someBindings[iBinding].descriptorCount > 1 ? VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT : 0u;

    VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
    extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    extendedInfo.pNext = nullptr;
    extendedInfo.bindingCount = (uint)someBindings.size();
    extendedInfo.pBindingFlags = bindingFlags.data();

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = &extendedInfo;
    createInfo.flags = 0u;
    createInfo.bindingCount = (uint)someBindings.size();
    createInfo.pBindings = someBindings.data();

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(platformVk->myDevice, &createInfo, nullptr, &layout));

    myCache[hash] = layout;
    return layout;
  }
//---------------------------------------------------------------------------//
  VkDescriptorSetLayout DescriptorSetLayoutCacheVk::TryGetDescriptorSetLayout(uint64 aHash)
  {
    std::lock_guard<std::mutex> lock(myCacheMutex);
    auto it = myCache.find(aHash);
    return it != myCache.end() ? it->second : nullptr;
  }
//---------------------------------------------------------------------------//
}

#endif