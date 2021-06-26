#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  /// <summary>
  /// Handles allocation/freeing of small temp descriptor pools that can be used during commandlist-recording for local resource bindings. The global/bindless resource descriptors are NOT handled with this class
  /// </summary>

  class DescriptorPoolAllocatorVk
  {
  public:
    DescriptorPoolAllocatorVk(uint aMaxNumDescriptors, uint aMaxNumSets);
    ~DescriptorPoolAllocatorVk();

    static VkDescriptorPool CreateDescriptorPool(uint aMaxNumDescriptors, uint aMaxNumSets);
    static VkDescriptorPool CreateDescriptorPool(uint aNumImages, uint aNumBuffers, uint aNumUniformBuffers, uint aNumSamplers, uint aMaxNumSets);

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