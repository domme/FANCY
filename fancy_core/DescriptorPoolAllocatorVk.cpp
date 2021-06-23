#include "fancy_core_precompile.h"
#include "DescriptorPoolAllocatorVk.h"
#include "RenderCore_PlatformVk.h"
#include "RenderCore.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  DescriptorPoolAllocatorVk::DescriptorPoolAllocatorVk(uint aMaxNumDescriptors, uint aMaxNumSets)
    : myMaxNumDescriptors(aMaxNumDescriptors)
    , myMaxNumSets(aMaxNumSets)
  {
    CreateAndAddDescriptorPool();
  }
//---------------------------------------------------------------------------//
  DescriptorPoolAllocatorVk::~DescriptorPoolAllocatorVk()
  {
    UpdateWaitingPools();
    ASSERT(myWaitingPools.empty());
    ASSERT(myAvaiablePools.size() == myCreatedPools.size());

    for (VkDescriptorPool pool : myCreatedPools)
      vkDestroyDescriptorPool(RenderCore::GetPlatformVk()->myDevice, pool, nullptr);
  }
 //---------------------------------------------------------------------------//
  VkDescriptorPool DescriptorPoolAllocatorVk::CreateDescriptorPool(uint aMaxNumDescriptors, uint aMaxNumSets)
  {
    return CreateDescriptorPool(aMaxNumDescriptors, aMaxNumDescriptors, aMaxNumDescriptors, aMaxNumDescriptors, aMaxNumSets);
  }
//---------------------------------------------------------------------------//
  VkDescriptorPool DescriptorPoolAllocatorVk::CreateDescriptorPool(
    uint aNumImages, uint aNumBuffers, uint aNumUniformBuffers, uint aNumSamplers, uint aMaxNumSets)
  {
    VkDescriptorPoolSize poolSizes[] =
    {
      { VK_DESCRIPTOR_TYPE_SAMPLER, aNumSamplers },
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, aNumImages },
      { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, aNumImages },
      { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, aNumImages},
      { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, aNumBuffers },
      { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, aNumBuffers },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, aNumUniformBuffers },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, aNumBuffers },
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, aNumUniformBuffers},
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, aNumBuffers },
    };

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.pNext = nullptr;
    poolCreateInfo.flags = 0u;
    poolCreateInfo.maxSets = aMaxNumSets;
    poolCreateInfo.pPoolSizes = poolSizes;
    poolCreateInfo.poolSizeCount = ARRAY_LENGTH(poolSizes);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkDescriptorPool descriptorPool;
    ASSERT_VK_RESULT(vkCreateDescriptorPool(platformVk->myDevice, &poolCreateInfo, nullptr, &descriptorPool));

    return descriptorPool;
  }
//---------------------------------------------------------------------------//
  VkDescriptorPool DescriptorPoolAllocatorVk::AllocateDescriptorPool()
  {
    UpdateWaitingPools();

    if (myAvaiablePools.empty())
      CreateAndAddDescriptorPool();

    ASSERT(!myAvaiablePools.empty());
    VkDescriptorPool pool = myAvaiablePools.front();
    myAvaiablePools.pop_front();

    return pool;
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::FreeDescriptorPool(VkDescriptorPool aDescriptorPool, uint64 aFence)
  {
#if FANCY_HEAVY_DEBUG
    for(auto& waiting : myWaitingPools)
      ASSERT(waiting.second != aDescriptorPool);
    for (VkDescriptorPool availablePool : myAvaiablePools)
      ASSERT(availablePool != aDescriptorPool);
    ASSERT(eastl::find(myCreatedPools.begin(), myCreatedPools.end(), aDescriptorPool) != myCreatedPools.end(), "Freed descriptorPool has never been created by this allocator");
#endif

    myWaitingPools.push_back({ aFence, aDescriptorPool });
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::UpdateWaitingPools()
  {
    for (auto it = myWaitingPools.begin(); it != myWaitingPools.end(); )
    {
      const eastl::pair<uint64, VkDescriptorPool>& waitingPool = *it;

      if (RenderCore::IsFenceDone(waitingPool.first))
      {
        VkDescriptorPool pool = waitingPool.second;
        ASSERT_VK_RESULT(vkResetDescriptorPool(RenderCore::GetPlatformVk()->myDevice, pool, 0u));

        myAvaiablePools.push_back(pool);
        it = myWaitingPools.erase(it);
      }
      else
        ++it;
    }
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::CreateAndAddDescriptorPool()
  {
    VkDescriptorPool descriptorPool = CreateDescriptorPool(myMaxNumDescriptors, myMaxNumSets);
    ASSERT(descriptorPool != nullptr);

    myCreatedPools.push_back(descriptorPool);
    myAvaiablePools.push_back(descriptorPool);
  }
//---------------------------------------------------------------------------//
}

#endif