#include "fancy_core_precompile.h"
#include "GpuResourceViewSetVk.h"
#include "DescriptorSetLayoutCacheVk.h"
#include "RenderCore_PlatformVk.h"
#include "GpuResourceViewDataVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  GpuResourceViewSetVk::GpuResourceViewSetVk(const eastl::span<GpuResourceViewRange>& someRanges)
    : GpuResourceViewSet(someRanges)
  {
    eastl::fixed_vector<VkDescriptorSetLayoutBinding, 32> setLayoutBindings;
    setLayoutBindings.resize(someRanges.size());
    myDescriptorTypePerRange.resize(someRanges.size());

    for (uint i = 0u; i < (uint)someRanges.size(); ++i)
    {
      VkDescriptorSetLayoutBinding& layoutBinding = setLayoutBindings[i];
      const GpuResourceViewRange& range = someRanges[i];
      if (range.myIsUnbounded)
        ASSERT(i == (uint)(someRanges.size() - 1), "Unbounded arrays can only appear as the last descriptor range in the set");

      ASSERT(!range.myResources.empty());

      GpuResourceView* firstResource = nullptr;
      for (const auto& resource : range.myResources)
      {
        if (resource)
        {
          firstResource = resource.get();
          break;
        }
      }
      ASSERT(firstResource, "There must be at least one valid resource set per range, otherwise the descriptorType can't be determined");

      VkDescriptorType descriptorType = RenderCore_PlatformVk::GetDescriptorType(firstResource);
      myDescriptorTypePerRange[i] = descriptorType;

      layoutBinding.binding = i;
      layoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
      layoutBinding.descriptorType = descriptorType;
      layoutBinding.pImmutableSamplers = nullptr;
      layoutBinding.descriptorCount = range.myIsUnbounded ? RenderCore_PlatformVk::MAX_DESCRIPTOR_ARRAY_SIZE : (uint) range.myResources.size();
    }

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    DescriptorSetLayoutCacheVk& setLayoutCache = platformVk->GetDescriptorSetLayoutCache();

    myDescriptorSetLayout = setLayoutCache.GetDescriptorSetLayout({ setLayoutBindings.begin(), setLayoutBindings.end() });
    VkDescriptorPool staticPool = platformVk->GetStaticDescriptorPool();

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &myDescriptorSetLayout;
    allocInfo.descriptorPool = staticPool;

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, &allocInfo, &descriptorSet);
    ASSERT(result == VK_SUCCESS);

    // Write the descriptors into the descriptor set
    VkWriteDescriptorSet baseWriteInfo = {};
    baseWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    baseWriteInfo.dstSet = descriptorSet;

    myDstImageLayoutPerRange.reserve(someRanges.size());
    myDstAccessFlagsPerRange.reserve(someRanges.size());
    
    eastl::fixed_vector<uint8, 256> rangeDatas;
    eastl::fixed_vector<VkWriteDescriptorSet, 64> writeInfos;
    for (uint i = 0u; i < (uint)someRanges.size(); ++i)
    {
      VkDescriptorType descriptorType = setLayoutBindings[i].descriptorType;

      VkAccessFlags rangeAccessFlags = VK_ACCESS_SHADER_READ_BIT;
      VkImageLayout rangeImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      switch (descriptorType)
      {
      case VK_DESCRIPTOR_TYPE_SAMPLER: ASSERT(false); break; // No support for samplers here
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break; // Not supported in HLSL
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        rangeAccessFlags = VK_ACCESS_SHADER_READ_BIT;
        rangeImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        rangeAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;
        rangeImageLayout = VK_IMAGE_LAYOUT_GENERAL;
        break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        rangeAccessFlags = VK_ACCESS_SHADER_READ_BIT;
        rangeImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        break;
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        rangeAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;
        rangeImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        break;
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        rangeAccessFlags = VK_ACCESS_UNIFORM_READ_BIT;
        rangeImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        break;
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        rangeAccessFlags = VK_ACCESS_SHADER_WRITE_BIT;
        rangeImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        break;
      default: ASSERT(false, "Missing implementation");
      }
      myDstImageLayoutPerRange.push_back(rangeImageLayout);
      myDstAccessFlagsPerRange.push_back(rangeAccessFlags);

      const GpuResourceViewRange& range = someRanges[i];
      uint startRangeDataIdx = (uint) rangeDatas.size();

      bool writeAllRangeViewsCombined = true;
      for (const SharedPtr<GpuResourceView>& view : range.myResources)
      {
        if (view == nullptr)
        {
          writeAllRangeViewsCombined = false;
          break;
        }

        for (uint iArrayElement = 0u; iArrayElement < range.myResources.size(); ++iArrayElement)
        {
          const SharedPtr<GpuResourceView>& view = range.myResources[iArrayElement];
          if (view == nullptr)
            continue;

          ASSERT(RenderCore_PlatformVk::GetDescriptorType(view.get()) == descriptorType);

          eastl::optional<VkDescriptorBufferInfo> descriptorBufferInfo;
          eastl::optional<VkDescriptorImageInfo> descriptorImageInfo;
          eastl::optional<VkBufferView> bufferView;
          RenderCore_PlatformVk::GetResourceViewDescriptorData(view.get(), descriptorType, descriptorBufferInfo, descriptorImageInfo, bufferView);

          uint64 dataSize = 0ull;
          void* dataPtr = nullptr;

          if (descriptorBufferInfo)
          {
            dataSize = sizeof(VkDescriptorBufferInfo);
            dataPtr = &descriptorBufferInfo.value();
          }
          else if (descriptorImageInfo)
          {
            dataSize = sizeof(VkDescriptorImageInfo);
            dataPtr = &descriptorImageInfo.value();
          }
          else
          {
            dataSize = sizeof(VkBufferView);
            dataPtr = &bufferView.value();
          }

          uint dstOffset = (uint) rangeDatas.size();
          rangeDatas.resize(rangeDatas.size() + dataSize);
          memcpy(rangeDatas.data() + dstOffset, dataPtr, dataSize);

          if (!writeAllRangeViewsCombined)
          {
            VkWriteDescriptorSet writeInfo = baseWriteInfo;
            writeInfo.descriptorType = descriptorType;
            writeInfo.dstArrayElement = iArrayElement;
            writeInfo.descriptorCount = 1;
            writeInfo.dstBinding = i;
            writeInfo.pImageInfo = reinterpret_cast<VkDescriptorImageInfo*>(rangeDatas.data()) + dstOffset;
            writeInfo.pBufferInfo = reinterpret_cast<VkDescriptorBufferInfo*>(rangeDatas.data()) + dstOffset;
            writeInfo.pTexelBufferView = reinterpret_cast<VkBufferView*>(rangeDatas.data()) + dstOffset;
            writeInfos.push_back(writeInfo);
          }
        }
      }

      if (writeAllRangeViewsCombined)
      {
        VkWriteDescriptorSet writeInfo = baseWriteInfo;
        writeInfo.descriptorType = descriptorType;
        writeInfo.dstArrayElement = 0u;
        writeInfo.descriptorCount = (uint) range.myResources.size();
        writeInfo.dstBinding = i;
        writeInfo.pImageInfo = reinterpret_cast<VkDescriptorImageInfo*>(rangeDatas.data()) + startRangeDataIdx;
        writeInfo.pBufferInfo = reinterpret_cast<VkDescriptorBufferInfo*>(rangeDatas.data()) + startRangeDataIdx;
        writeInfo.pTexelBufferView = reinterpret_cast<VkBufferView*>(rangeDatas.data()) + startRangeDataIdx;
        writeInfos.push_back(writeInfo);
      }
    }

    vkUpdateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, (uint)writeInfos.size(), writeInfos.data(), 0u, nullptr);
  }
//---------------------------------------------------------------------------//
  GpuResourceViewSetVk::~GpuResourceViewSetVk()
  {
  }
//---------------------------------------------------------------------------//
}

#endif


