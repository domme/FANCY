#include "fancy_core_precompile.h"
#include "DescriptorPoolAllocatorVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  DescriptorPoolAllocatorVk::DescriptorPoolAllocatorVk(uint aMaxNumDescriptors, uint aMaxNumSets)
    : myMaxNumDescriptors(aMaxNumDescriptors)
    , myMaxNumSets(aMaxNumSets)
  {
    AddNewDescriptorPool();
  }
//---------------------------------------------------------------------------//
  DescriptorPoolAllocatorVk::~DescriptorPoolAllocatorVk()
  {
  }
//---------------------------------------------------------------------------//
  VkDescriptorPool DescriptorPoolAllocatorVk::AllocateDescriptorPool()
  {
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::FreeDescriptorPool(VkDescriptorPool aDescriptorSet, uint64 aFence)
  {
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::UpdateWaitingPools()
  {
    for (uint i = 0u; i < myWaitingPools.Size(); ++i)
    {
      const std::pair<uint64, VkDescriptorPool>& waitingPool = myWaitingPools[i];


    }
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::AddNewDescriptorPool()
  {
    VkDescriptorPoolSize poolSizes[] =
    {
      { VK_DESCRIPTOR_TYPE_SAMPLER, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, myMaxNumDescriptors },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, myMaxNumDescriptors },
    };

    VkDescriptorPoolCreateInfo poolCreateInfo;
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.pNext = nullptr;
    poolCreateInfo.flags = 0u;
    poolCreateInfo.maxSets = myMaxNumSets;
    poolCreateInfo.pPoolSizes = poolSizes;
    poolCreateInfo.poolSizeCount = ARRAY_LENGTH(poolSizes);
    
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkDescriptorPool descriptorPool;
    ASSERT_VK_RESULT(vkCreateDescriptorPool(platformVk->myDevice, &poolCreateInfo, nullptr, &descriptorPool));

    myCreatedPools.Add(descriptorPool);
    myAvaiablePools.Add(descriptorPool);
  }
//---------------------------------------------------------------------------//
}

