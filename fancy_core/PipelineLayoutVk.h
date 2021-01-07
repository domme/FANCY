#pragma once

#include "VkPrerequisites.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct PipelineLayoutCreateInfoVk
  {
    struct DescriptorSetInfo
    {
      eastl::fixed_vector<VkDescriptorSetLayoutBinding, 32> myBindings;
    };

    eastl::fixed_vector<DescriptorSetInfo, 16> myDescriptorSetInfos;
  };

  struct PipelineLayoutVk
  {
    PipelineLayoutVk(VkPipelineLayout aLayout, const PipelineLayoutCreateInfoVk& aCreateInfo, const eastl::span<VkDescriptorSetLayout>& someDescriptorSetLayouts);
    ~PipelineLayoutVk();

    struct DescriptorSet
    {
      eastl::fixed_vector<VkDescriptorSetLayoutBinding, 32> myBindings;
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
      bool myHasAllDescriptorsBound = false;
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

      // Returns true if the data has changed
      template<class T>
      bool Set(const T& aDescriptor, uint anIndex, bool aCheckHoles  = true)
      {
        ASSERT(sizeof(T) == GetElementSize());
        ASSERT(anIndex < Size());

        bool hasChanged = false;

        T& data = *((T*)myData.data() + anIndex);
        if (memcmp(&data, &aDescriptor, sizeof(T)) != 0)
        {
          data = aDescriptor;
          hasChanged = true;
        }

        if (aCheckHoles)
        {
          myHasAllDescriptorsBound = true;
          for (uint i = 0u; i < Size(); ++i)
          {
            if (IsEmptyDescriptor<T>(i))
            {
              myHasAllDescriptorsBound = false;
              break;
            }
          }
        }

        return hasChanged;
      }

      template<class T>
      bool IsEmptyDescriptor(uint anIndex)
      {
        ASSERT(sizeof(T) == GetElementSize());
        ASSERT(anIndex < Size());

        constexpr uint8 emptyElement[sizeof(T)] = { 0u };
        return memcmp((T*)myData.data() + anIndex, emptyElement, sizeof(T)) == 0;
      }
    };

    struct DescriptorSet
    {
      mutable bool myIsDirty = true;
      bool myHasUnboundedBindings = false;
      uint myNumBoundedDescriptors = 0u;
      VkDescriptorSetLayout myLayout = nullptr;
      eastl::fixed_vector<DescriptorRange, 8> myRanges;
      // Used for binding pre-uploaded descriptor sets from a GpuResourceViewSet. In this case, myBindings are ignored
      VkDescriptorSet myConstantDescriptorSet = nullptr;
    };

    eastl::fixed_vector<DescriptorSet, 8> myDescriptorSets;
    bool myIsDirty = true;

    void Clear();
  };

}

#endif

