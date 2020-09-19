#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct PipelineLayoutCreateInfoVk
  {
    struct DescriptorSetInfo
    {
      eastl::fixed_vector<VkDescriptorSetLayoutBinding, 32> myRanges;
    };

    eastl::fixed_vector<DescriptorSetInfo, 16> myDescriptorSetInfos;
  };

  struct PipelineLayoutVk
  {
    PipelineLayoutVk(VkPipelineLayout aLayout, const PipelineLayoutCreateInfoVk& aCreateInfo, const eastl::span<VkDescriptorSetLayout>& someDescriptorSetLayouts);
    ~PipelineLayoutVk();

    struct DescriptorSet
    {
      eastl::fixed_vector<VkDescriptorSetLayoutBinding, 32> myRanges;
      VkDescriptorSetLayout myLayout;
    };

    eastl::fixed_vector<DescriptorSet, 8> myDescriptorSets;

    VkPipelineLayout myPipelineLayout;
  };

  struct PipelineLayoutBindingsVk
  {
    PipelineLayoutBindingsVk(const PipelineLayoutVk& aLayout);

    union DescriptorData
    {
      VkDescriptorBufferInfo myBufferInfo;
      VkDescriptorImageInfo myImageInfo;
      VkBufferView myTexelBufferView;
    };

    struct DescriptorRange
    {
      bool myIsUnbounded = false;
      VkDescriptorType myType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
      uint myElementSize = 0u;
      eastl::fixed_vector<uint8, sizeof(DescriptorData) * 8> myData;

      uint GetElementSize() const;
      uint Size() const;

      void ResizeUp(uint aNewSize);

      template<class T>
      T& Get(uint anIndex)
      {
        ASSERT(sizeof(T) == GetElementSize());
        ASSERT(anIndex < Size());

        return *((T*)myData.data() + anIndex);
      }

      template<class T>
      void Set(const T& aDescriptor, uint anIndex)
      {
        ASSERT(sizeof(T) == GetElementSize());
        ASSERT(anIndex < Size());

        *((T*)myData.data() + anIndex) = aDescriptor;
      }
    };

    struct DescriptorSet
    {
      mutable bool myIsDirty = true;
      bool myHasUnboundedRanges = false;
      uint myNumBoundedDescriptors = 0u;
      VkDescriptorSetLayout myLayout = nullptr;
      eastl::fixed_vector<DescriptorRange, 8> myRanges;
    };

    eastl::fixed_vector<DescriptorSet, 8> myDescriptorSets;

    void Clear();
  };

}

#endif

