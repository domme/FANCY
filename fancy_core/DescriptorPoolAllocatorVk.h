#pragma once

#include "RenderEnums.h"
#include "VkPrerequisites.h"
#include "StaticArray.h"

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

    StaticArray<VkDescriptorPool, 16> myCreatedPools;
    StaticArray<VkDescriptorPool, 16> myAvaiablePools;
    StaticArray<std::pair<uint64, VkDescriptorPool>, 16> myWaitingPools;
  };
}



