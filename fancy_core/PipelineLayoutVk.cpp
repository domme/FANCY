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
      set.myRanges = aCreateInfo.myDescriptorSetInfos[i].myRanges;
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
  {
    myDescriptorSets.resize(aLayout.myDescriptorSets.size());

    for (uint iSet = 0u; iSet < (uint) myDescriptorSets.size(); ++iSet)
    {
      DescriptorSet& bindingSet = myDescriptorSets[iSet];
      const PipelineLayoutVk::DescriptorSet& layoutSet = aLayout.myDescriptorSets[iSet];

      bindingSet.myIsDirty = true;
      bindingSet.myLayout = layoutSet.myLayout;

      uint numRangesRequired = 0u;
      for (const VkDescriptorSetLayoutBinding& layoutRange : layoutSet.myRanges)
        numRangesRequired = glm::max(numRangesRequired, layoutRange.binding);

      bindingSet.myRanges.resize(numRangesRequired);

      bool hasUnboundedRanges = false;
      for (uint iRange = 0u; iRange < (uint)layoutSet.myRanges.size(); ++iRange)
      {
        const VkDescriptorSetLayoutBinding& layoutRange = layoutSet.myRanges[iRange];
        DescriptorRange& bindingRange = bindingSet.myRanges[layoutRange.binding];

        bindingRange.myType = layoutRange.descriptorType;
        bindingRange.myElementSize = bindingRange.GetElementSize();

        if (layoutRange.descriptorCount == UINT_MAX) // TODO: Check if this is really the case for unbounded arrays
        {
          bindingRange.myIsUnbounded = true;
          hasUnboundedRanges = true;
        }
        else
        {
          bindingRange.ResizeUp(layoutRange.descriptorCount);
          memset(bindingRange.myData.data(), 0u, DYN_ARRAY_BYTESIZE(bindingRange.myData));
        }
      }

      bindingSet.myNumBoundedDescriptors = 0u;
      bindingSet.myHasUnboundedRanges = hasUnboundedRanges;

      if (!hasUnboundedRanges)
      {
        for (const DescriptorRange& range : bindingSet.myRanges)
          bindingSet.myNumBoundedDescriptors += (uint)range.Size();
      }
    }
  }

  uint PipelineLayoutBindingsVk::DescriptorRange::GetElementSize() const
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
  uint PipelineLayoutBindingsVk::DescriptorRange::Size() const
  {
    return (uint)myData.size() / GetElementSize();
  }
  //---------------------------------------------------------------------------//
  void PipelineLayoutBindingsVk::DescriptorRange::ResizeUp(uint aNewSize)
  {
    myData.resize(glm::max((uint)myData.size(), aNewSize * GetElementSize()));
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  void PipelineLayoutBindingsVk::Clear()
  {
    for (DescriptorSet& set : myDescriptorSets)
    {
      set.myIsDirty = true;
      for (DescriptorRange& range : set.myRanges)
        range.myData.clear();
    }
  }
//---------------------------------------------------------------------------//
}


#endif