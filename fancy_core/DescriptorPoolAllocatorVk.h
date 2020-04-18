#pragma once

#include "VkPrerequisites.h"
#include "StaticArray.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  class DescriptorPoolAllocatorVk
  {
  public:
    DescriptorPoolAllocatorVk(uint aMaxNumDescriptors, uint aMaxNumSets);
    ~DescriptorPoolAllocatorVk();

    VkDescriptorPool AllocateDescriptorPool();
    void FreeDescriptorPool(VkDescriptorPool aDescriptorPool, uint64 aFence);

  private:
    void UpdateWaitingPools();
    void CreateDescriptorPool();

    uint myMaxNumDescriptors;
    uint myMaxNumSets;

    StaticArray<VkDescriptorPool, 128> myCreatedPools;
    StaticArray<VkDescriptorPool, 128> myAvaiablePools;
    StaticArray<std::pair<uint64, VkDescriptorPool>, 128> myWaitingPools;
  };
}

#endif