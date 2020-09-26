#include "fancy_core_precompile.h"
#include "PipelineLayoutVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  PipelineLayoutVk::PipelineLayoutVk(
    VkPipelineLayout aLayout, 
    const PipelineLayoutCreateInfoVk& aCreateInfo,
    const eastl::span<VkDescriptorSetLayout>& someDescriptorSetLayouts)
      : myPipelineLayout(aLayout)
  {
    ASSERT(aCreateInfo.myDescriptorSetInfos.size() == someDescriptorSetLayouts.size());

    myDescriptorSets.resize(aCreateInfo.myDescriptorSetInfos.size());

    for (uint i = 0u; i < (uint)myDescriptorSets.size(); ++i)
    {
      DescriptorSet& set = myDescriptorSets[i];
      set.myBindings = aCreateInfo.myDescriptorSetInfos[i].myBindings;
      set.myLayout = someDescriptorSetLayouts[i];
    }
  }
//---------------------------------------------------------------------------//
  PipelineLayoutVk::~PipelineLayoutVk()
  {
    vkDestroyPipelineLayout(RenderCore::GetPlatformVk()->GetDevice(), myPipelineLayout, nullptr);
    // DescriptorSetLayouts are not destroyed here because they are managed by the PipelineLayoutCacheVk
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  PipelineLayoutBindingsVk::PipelineLayoutBindingsVk(const PipelineLayoutVk& aLayout)
    : myIsDirty(true)
  {
    myDescriptorSets.resize(aLayout.myDescriptorSets.size());

    for (uint iSet = 0u; iSet < (uint) myDescriptorSets.size(); ++iSet)
    {
      DescriptorSet& dstSet = myDescriptorSets[iSet];
      const PipelineLayoutVk::DescriptorSet& srcSet = aLayout.myDescriptorSets[iSet];

      dstSet.myIsDirty = true;
      dstSet.myLayout = srcSet.myLayout;

      uint numRangesRequired = 0u;
      for (const VkDescriptorSetLayoutBinding& layoutRange : srcSet.myBindings)
        numRangesRequired = glm::max(numRangesRequired, layoutRange.binding + 1);

      dstSet.myBindings.resize(numRangesRequired);

      bool hasUnboundedBindings = false;
      for (uint iBinding = 0u; iBinding < (uint)srcSet.myBindings.size(); ++iBinding)
      {
        const VkDescriptorSetLayoutBinding& srcBinding = srcSet.myBindings[iBinding];
        DescriptorBinding& dstBinding = dstSet.myBindings[srcBinding.binding];

        dstBinding.myType = srcBinding.descriptorType;
        dstBinding.myElementSize = dstBinding.GetElementSize();

        if (srcSet.myBindings[iBinding].descriptorCount == RenderCore_PlatformVk::MAX_DESCRIPTOR_ARRAY_SIZE)
        {
          dstBinding.myIsUnbounded = true;
          hasUnboundedBindings = true;
        }
        else
        {
          dstBinding.ResizeUp(srcBinding.descriptorCount);
          memset(dstBinding.myData.data(), 0u, DYN_ARRAY_BYTESIZE(dstBinding.myData));
        }
      }

      dstSet.myNumBoundedDescriptors = 0u;
      dstSet.myHasUnboundedBindings = hasUnboundedBindings;

      if (!hasUnboundedBindings)
      {
        for (const DescriptorBinding& range : dstSet.myBindings)
          dstSet.myNumBoundedDescriptors += (uint)range.Size();
      }
    }
  }

  uint PipelineLayoutBindingsVk::DescriptorBinding::GetElementSize() const
  {
    switch (myType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return sizeof(VkDescriptorImageInfo);
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return sizeof(VkBufferView);
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return sizeof(VkDescriptorBufferInfo);
    default: ASSERT(false); return 1u;
    }
  }
  //---------------------------------------------------------------------------//
  uint PipelineLayoutBindingsVk::DescriptorBinding::Size() const
  {
    return (uint)myData.size() / GetElementSize();
  }
  //---------------------------------------------------------------------------//
  void PipelineLayoutBindingsVk::DescriptorBinding::ResizeUp(uint aNewSize)
  {
    myData.resize(glm::max((uint)myData.size(), aNewSize * GetElementSize()));
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void PipelineLayoutBindingsVk::Clear()
  {
    myIsDirty = true;
    for (DescriptorSet& set : myDescriptorSets)
    {
      set.myIsDirty = true;
      for (DescriptorBinding& range : set.myBindings)
      {
        if (range.myIsUnbounded)
          range.myData.clear();
        else
          memset(range.myData.data(), 0, DYN_ARRAY_BYTESIZE(range.myData));
      }
    }
  }
//---------------------------------------------------------------------------//
}


#endif