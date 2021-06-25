#pragma once

#include "FancyCoreDefines.h"

#if FANCY_ENABLE_VK

#include "RenderEnums.h"
#include "VkPrerequisites.h"
#include "PagedLinearAllocator.h"
#include "GlobalDescriptorAllocation.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
// Manages the allocation and lifetime of global ("bindless") resource- and sampler descriptors.
// Freed descriptors are only reused after the frame the free was requested in has been fully processed by the GPU
//---------------------------------------------------------------------------//
  class GlobalDescriptorSetVk
  {
  public:
    GlobalDescriptorSetVk(const RenderPlatformProperties& someProperties);
    ~GlobalDescriptorSetVk();

    GlobalDescriptorAllocation AllocateAndWriteDescriptor(GlobalResourceType aType, const VkDescriptorImageInfo& anImageInfo, const char* aDebugName = nullptr);
    GlobalDescriptorAllocation AllocateAndWriteDescriptor(GlobalResourceType aType, const VkDescriptorBufferInfo& aBufferInfo, const char* aDebugName = nullptr);
    void FreeDescriptorAfterFrameDone(const GlobalDescriptorAllocation& aDescriptor);

    void ProcessGlobalDescriptorFrees();

    VkDescriptorSet GetDescriptorSet() const { return myDescriptorSet; }

  private:
    GlobalDescriptorAllocation AllocateAndWriteDescriptor(GlobalResourceType aType, const VkDescriptorImageInfo* anImageInfo, const VkDescriptorBufferInfo* aBufferInfo, const char* aDebugName = nullptr);
  
    // Dedicated descriptor pool that is used just for this global descriptor set
    VkDescriptorPool myDescriptorPool;
    // Global descriptor set that holds all active resource-views and samplers 
    VkDescriptorSet myDescriptorSet;

    uint myNumGlobalDescriptors[GLOBAL_RESOURCE_NUM];
    PagedLinearAllocator myAllocators[GLOBAL_RESOURCE_NUM];

    static constexpr uint ourNumGlobalFreeLists = RenderCore::Constants::NUM_QUEUED_FRAMES + 1;
    eastl::fixed_vector<GlobalDescriptorAllocation, 256> myDescriptorsToFree[ourNumGlobalFreeLists];
  };
//---------------------------------------------------------------------------//
}

#endif