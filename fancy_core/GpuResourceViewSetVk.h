#pragma once
#include "GpuResourceViewSet.h"
#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuResourceViewSetVk : public GpuResourceViewSet
  {
  public:
    GpuResourceViewSetVk(const eastl::span<GpuResourceViewRange>& someRanges);
    ~GpuResourceViewSetVk() override;

    VkDescriptorType GetDescriptorType(uint aRangeIdx) const { return myDescriptorTypePerRange[aRangeIdx]; }
    VkImageLayout GetDstImageLayout(uint aRangeIdx) const { return myDstImageLayoutPerRange[aRangeIdx]; }
    VkAccessFlags GetDstAccessFlags(uint aRangeIdx) const { return myDstAccessFlagsPerRange[aRangeIdx]; }
    VkDescriptorSet GetDescriptorSet() const { return myDescriptorSet; }

  protected:
    VkDescriptorSet myDescriptorSet = nullptr;
    VkDescriptorSetLayout myDescriptorSetLayout = nullptr;
    eastl::fixed_vector<VkDescriptorType, 32> myDescriptorTypePerRange;
    eastl::fixed_vector<VkImageLayout, 32> myDstImageLayoutPerRange;
    eastl::fixed_vector<VkAccessFlags, 32> myDstAccessFlagsPerRange;
  };
//---------------------------------------------------------------------------//
}

#endif

