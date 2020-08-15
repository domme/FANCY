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
    CreateDescriptorPool();
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
  VkDescriptorPool DescriptorPoolAllocatorVk::AllocateDescriptorPool()
  {
    UpdateWaitingPools();

    if (myAvaiablePools.empty())
      CreateDescriptorPool();

    ASSERT(!myAvaiablePools.empty());
    VkDescriptorPool pool = myAvaiablePools.front();
    myAvaiablePools.pop_front();

    return pool;
  }
//---------------------------------------------------------------------------//
  void DescriptorPoolAllocatorVk::FreeDescriptorPool(VkDescriptorPool aDescriptorPool, uint64 aFence)
  {
#if FANCY_RENDERER_DEBUG
    for(auto& waiting : myWaitingPools)
      ASSERT(waiting.second != aDescriptorPool);
    for (VkDescriptorPool availablePool : myAvaiablePools)
      ASSERT(availablePool != aDescriptorPool);
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
  void DescriptorPoolAllocatorVk::CreateDescriptorPool()
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

    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.pNext = nullptr;
    poolCreateInfo.flags = 0u;
    poolCreateInfo.maxSets = myMaxNumSets;
    poolCreateInfo.pPoolSizes = poolSizes;
    poolCreateInfo.poolSizeCount = ARRAY_LENGTH(poolSizes);
    
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkDescriptorPool descriptorPool;
    ASSERT_VK_RESULT(vkCreateDescriptorPool(platformVk->myDevice, &poolCreateInfo, nullptr, &descriptorPool));

    myCreatedPools.push_back(descriptorPool);
    myAvaiablePools.push_back(descriptorPool);
  }
//---------------------------------------------------------------------------//
}

#endif