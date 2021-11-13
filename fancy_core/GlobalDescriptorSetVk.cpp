#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "GlobalDescriptorSetVk.h"

#include "DescriptorPoolAllocatorVk.h"
#include "PipelineLayoutVk.h"
#include "RenderCore_PlatformVk.h"
#include "TimeManager.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  GlobalDescriptorSetVk::GlobalDescriptorSetVk(const RenderPlatformProperties& someProperties)
  {
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
    {
      const uint numDescriptors = RenderCore::GetNumDescriptors(static_cast<GlobalResourceType>(i), someProperties);
      myNumGlobalDescriptors[i] = numDescriptors;
      myAllocators[i] = PagedLinearAllocator(numDescriptors);
    }

    const uint numImageDescriptors =
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_1D] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_1D_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_1D_INT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_2D] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_2D_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_2D_INT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_3D] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_3D_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_3D_INT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_CUBE] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_CUBE_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_TEXTURE_CUBE_INT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_1D] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_1D_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_1D_INT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_2D] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_2D_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_2D_INT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_3D] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_3D_UINT] +
      myNumGlobalDescriptors[GLOBAL_RESOURCE_RWTEXTURE_3D_INT];

    const uint numBufferDescriptors =
      myNumGlobalDescriptors[GLOBAL_RESOURCE_BUFFER] + myNumGlobalDescriptors[GLOBAL_RESOURCE_RWBUFFER];

    const uint numSamplerDescriptors = myNumGlobalDescriptors[GLOBAL_RESOURCE_SAMPLER];

    const uint numAccelerationStructures = myNumGlobalDescriptors[GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE];

    myDescriptorPool = DescriptorPoolAllocatorVk::CreateDescriptorPool(
      glm::max(1u, numImageDescriptors), 
      glm::max(1u, numBufferDescriptors),
      glm::max(1u, numAccelerationStructures),
      1u, 
      glm::max(1u, numSamplerDescriptors), 1);
    ASSERT(myDescriptorPool);

    // Allocate the one single descriptor set from the pool.
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = myDescriptorPool;
    allocInfo.pSetLayouts = &platformVk->GetPipelineLayout()->myDescriptorSetLayout_GlobalResourcesSamplers;
    allocInfo.descriptorSetCount = 1;
    ASSERT_VK_RESULT(vkAllocateDescriptorSets(platformVk->GetDevice(), &allocInfo, &myDescriptorSet));

    // Initialize with null-descriptors
    VkDescriptorImageInfo nullImageDescriptor;
    nullImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    nullImageDescriptor.imageView = VK_NULL_HANDLE;
    nullImageDescriptor.sampler = VK_NULL_HANDLE;

    VkDescriptorBufferInfo nullBufferDescriptor;
    nullBufferDescriptor.buffer = VK_NULL_HANDLE;
    nullBufferDescriptor.offset = 0;
    nullBufferDescriptor.range = VK_WHOLE_SIZE;

    uint maxNumDescriptors = 0;
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
      maxNumDescriptors = glm::max(maxNumDescriptors, myNumGlobalDescriptors[i]);

    eastl::vector<VkDescriptorImageInfo> nullImages(maxNumDescriptors, nullImageDescriptor);
    eastl::vector<VkDescriptorBufferInfo> nullBuffers(maxNumDescriptors, nullBufferDescriptor);
    eastl::vector<VkBufferView> nullBufferViews(maxNumDescriptors, VK_NULL_HANDLE);
    eastl::vector<VkAccelerationStructureKHR> nullAsViews(maxNumDescriptors, VK_NULL_HANDLE);

    eastl::fixed_vector<VkWriteDescriptorSet, GLOBAL_RESOURCE_NUM> descSetWrites;
    VkWriteDescriptorSetAccelerationStructureKHR asWriteInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
    asWriteInfo.accelerationStructureCount = myNumGlobalDescriptors[GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE];
    asWriteInfo.pAccelerationStructures = nullAsViews.data();
    
    for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
    {
      if (i == GLOBAL_RESOURCE_SAMPLER)
        continue; // Apparently there is no such thing as a null-descriptor for samplers. The debug-layer complains

      VkWriteDescriptorSet writeInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
      writeInfo.pNext = i == GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE ? &asWriteInfo : nullptr;
      writeInfo.descriptorType = RenderCore_PlatformVk::GetDescriptorType(static_cast<GlobalResourceType>(i));
      writeInfo.descriptorCount = myNumGlobalDescriptors[i];
      writeInfo.dstSet = myDescriptorSet;
      writeInfo.dstBinding = i;
      writeInfo.dstArrayElement = 0;
      writeInfo.pBufferInfo = nullBuffers.data();
      writeInfo.pImageInfo = nullImages.data();
      writeInfo.pTexelBufferView = nullBufferViews.data();

      descSetWrites.push_back(writeInfo);
    }

    vkUpdateDescriptorSets(platformVk->GetDevice(), (uint)descSetWrites.size(), descSetWrites.data(), 0, nullptr);
  }
//---------------------------------------------------------------------------//
  GlobalDescriptorSetVk::~GlobalDescriptorSetVk()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkDestroyDescriptorPool(platformVk->GetDevice(), myDescriptorPool, nullptr);
  }
//---------------------------------------------------------------------------//
  GlobalDescriptorAllocation GlobalDescriptorSetVk::AllocateAndWriteRTASDescriptor(VkAccelerationStructureKHR anAccelerationStructure, const char* aDebugName)
  {
    return AllocateAndWriteDescriptor(GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE, nullptr, nullptr, anAccelerationStructure, aDebugName);
  }
//---------------------------------------------------------------------------//
  GlobalDescriptorAllocation GlobalDescriptorSetVk::AllocateAndWriteDescriptor(GlobalResourceType aType, const VkDescriptorImageInfo& anImageInfo, const char* aDebugName)
  {
    ASSERT(aType != GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE);
    return AllocateAndWriteDescriptor(aType, &anImageInfo, nullptr, nullptr, aDebugName);
  }
//---------------------------------------------------------------------------//
  GlobalDescriptorAllocation GlobalDescriptorSetVk::AllocateAndWriteDescriptor(GlobalResourceType aType, const VkDescriptorBufferInfo& aBufferInfo, const char* aDebugName)
  {
    ASSERT(aType != GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE);
    return AllocateAndWriteDescriptor(aType, nullptr, &aBufferInfo, nullptr, aDebugName);
  }
//---------------------------------------------------------------------------//
  GlobalDescriptorAllocation GlobalDescriptorSetVk::AllocateAndWriteDescriptor(GlobalResourceType aType,     
    const VkDescriptorImageInfo* anImageInfo, const VkDescriptorBufferInfo* aBufferInfo, VkAccelerationStructureKHR anAccelerationStructure, const char* aDebugName)
  {
    ASSERT(myAllocators[aType].GetPageSize() != 0);

    uint64 offset;
    const PagedLinearAllocator::Page* page = myAllocators[aType].Allocate(1, 1, offset, aDebugName);
    ASSERT(page, "Failed allocating shader-visible descriptor. Consider increasing the max Global descriptor sizes");
    
    VkWriteDescriptorSet writeInfo = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    writeInfo.descriptorType = RenderCore_PlatformVk::GetDescriptorType(aType);
    writeInfo.descriptorCount = 1;
    writeInfo.dstSet = myDescriptorSet;
    writeInfo.dstBinding = aType;
    writeInfo.dstArrayElement = static_cast<uint>(offset);
    writeInfo.pImageInfo = anImageInfo;
    writeInfo.pBufferInfo = aBufferInfo;
    writeInfo.pTexelBufferView = nullptr;
    
    VkWriteDescriptorSetAccelerationStructureKHR asWriteInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
    if (anAccelerationStructure)
    {
      ASSERT(aType == GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE);
      asWriteInfo.accelerationStructureCount = 1;
      asWriteInfo.pAccelerationStructures = &anAccelerationStructure;
      writeInfo.pNext = &asWriteInfo;
    }
    
    vkUpdateDescriptorSets(RenderCore::GetPlatformVk()->GetDevice(), 1, &writeInfo, 0, nullptr);

    GlobalDescriptorAllocation allocation;
    allocation.myResourceType = aType;
    allocation.myIndex = static_cast<uint>(offset);
    return allocation;
  }
//---------------------------------------------------------------------------//
  void GlobalDescriptorSetVk::FreeDescriptorAfterFrameDone(const GlobalDescriptorAllocation& aDescriptor)
  {
    ASSERT(myAllocators[aDescriptor.myResourceType].GetPageSize() != 0);
    ASSERT(aDescriptor.myIndex < myNumGlobalDescriptors[aDescriptor.myResourceType]);

    const uint freeListIdx = Time::ourFrameIdx % ourNumGlobalFreeLists;
    myDescriptorsToFree[freeListIdx].push_back(aDescriptor);
  }
//---------------------------------------------------------------------------//
  void GlobalDescriptorSetVk::ProcessGlobalDescriptorFrees()
  {
    if (Time::ourFrameIdx < ourNumGlobalFreeLists - 1)
      return;

    const uint64 frameIdxToProcess = Time::ourFrameIdx - (static_cast<uint64>(ourNumGlobalFreeLists) - 1u);
    ASSERT(RenderCore::IsFrameDone(frameIdxToProcess));

    const uint freeListToProcess = frameIdxToProcess % ourNumGlobalFreeLists;

#if FANCY_HEAVY_DEBUG
    VkDescriptorImageInfo nullImageDescriptor;
    nullImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    nullImageDescriptor.imageView = VK_NULL_HANDLE;
    nullImageDescriptor.sampler = VK_NULL_HANDLE;

    VkDescriptorBufferInfo nullBufferDescriptor;
    nullBufferDescriptor.buffer = VK_NULL_HANDLE;
    nullBufferDescriptor.offset = 0;
    nullBufferDescriptor.range = VK_WHOLE_SIZE;

    VkBufferView nullBufferView = VK_NULL_HANDLE;

    VkAccelerationStructureKHR nullAs = VK_NULL_HANDLE;

    eastl::vector<VkWriteDescriptorSet> writeInfos;
    writeInfos.reserve(myDescriptorsToFree[freeListToProcess].size());

    eastl::vector<VkWriteDescriptorSetAccelerationStructureKHR> asWriteInfos;
    asWriteInfos.reserve(myDescriptorsToFree[freeListToProcess].size());

#endif

    for (const GlobalDescriptorAllocation& descriptorToFree : myDescriptorsToFree[freeListToProcess])
    {
      const GlobalResourceType resourceType = descriptorToFree.myResourceType;
      myAllocators[resourceType].Free({ descriptorToFree.myIndex, descriptorToFree.myIndex + 1 });

#if FANCY_HEAVY_DEBUG
      if (resourceType != GLOBAL_RESOURCE_SAMPLER)
      {
        // Replace with null descriptor
        // For debugging, this could also become a special error-resource for detecting if something deleted is accessed in a shader.
        VkWriteDescriptorSet& writeInfo = writeInfos.push_back();

        writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeInfo.pNext = nullptr;
        writeInfo.dstSet = myDescriptorSet;
        writeInfo.dstBinding = static_cast<uint>(descriptorToFree.myResourceType);
        writeInfo.dstArrayElement = descriptorToFree.myIndex;
        writeInfo.descriptorType = RenderCore_PlatformVk::GetDescriptorType(descriptorToFree.myResourceType);
        writeInfo.descriptorCount = 1;
        writeInfo.pBufferInfo = &nullBufferDescriptor;
        writeInfo.pImageInfo = &nullImageDescriptor;
        writeInfo.pTexelBufferView = &nullBufferView;
        
        if (resourceType == GLOBAL_RESOURCE_RT_ACCELERATION_STRUCTURE)
        {
          VkWriteDescriptorSetAccelerationStructureKHR& asWriteInfo = asWriteInfos.push_back();
          asWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
          asWriteInfo.pNext = nullptr;
          asWriteInfo.accelerationStructureCount = 1;
          asWriteInfo.pAccelerationStructures = &nullAs;
          writeInfo.pNext = &asWriteInfo;
        }
      }
#endif
    }

#if FANCY_HEAVY_DEBUG
    vkUpdateDescriptorSets(RenderCore::GetPlatformVk()->GetDevice(), static_cast<uint>(writeInfos.size()), writeInfos.data(), 0, nullptr);
#endif

    myDescriptorsToFree[freeListToProcess].clear();
  }
//---------------------------------------------------------------------------//
}

#endif