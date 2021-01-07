#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class DescriptorPoolAllocatorVk
  {
  public:
    DescriptorPoolAllocatorVk(uint aMaxNumDescriptors, uint aMaxNumSets);
    ~DescriptorPoolAllocatorVk();

    static VkDescriptorPool CreateDescriptorPool(uint aMaxNumDescriptors, uint aMaxNumSets);

    VkDescriptorPool AllocateDescriptorPool();
    void FreeDescriptorPool(VkDescriptorPool aDescriptorPool, uint64 aFence);

  private:
    void UpdateWaitingPools();
    void CreateAndAddDescriptorPool();

    uint myMaxNumDescriptors;
    uint myMaxNumSets;

    eastl::fixed_vector<VkDescriptorPool, 64> myCreatedPools;
    eastl::fixed_list<VkDescriptorPool, 64> myAvaiablePools;
    eastl::fixed_list<eastl::pair<uint64, VkDescriptorPool>, 64> myWaitingPools;
  };
}

#endif