#include "fancy_core_precompile.h"
#include "PipelineLayoutVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "RenderUtils.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace
  {
    VkDescriptorSetLayout locCreateDescriptorSetLayout(const VkDescriptorSetLayoutBinding* someBindings, uint aNumBindings)
    {
      eastl::fixed_vector<VkDescriptorBindingFlags, 16> bindingFlags;
      bindingFlags.resize(aNumBindings);

      // Always allow for arrays to be partially bound
      for (uint iBinding = 0u; iBinding < aNumBindings; ++iBinding)
        bindingFlags[iBinding] = someBindings[iBinding].descriptorCount > 1 ? VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT : 0u;

      VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
      extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
      extendedInfo.pNext = nullptr;
      extendedInfo.bindingCount = aNumBindings;
      extendedInfo.pBindingFlags = bindingFlags.data();

      VkDescriptorSetLayoutCreateInfo createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.pNext = &extendedInfo;
      createInfo.flags = 0u;
      createInfo.bindingCount = aNumBindings;
      createInfo.pBindings = someBindings;

      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

      VkDescriptorSetLayout layout = nullptr;
      ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(platformVk->myDevice, &createInfo, nullptr, &layout));

      return layout;
    }
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  PipelineLayoutVk::PipelineLayoutVk(const RenderPlatformProperties& someProperties)
    : myPipelineLayout(nullptr)
  {
    // Resource- & Sampler-descriptor set
    {
      VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)alloca(sizeof(VkDescriptorSetLayoutBinding) * GLOBAL_RESOURCE_NUM);
      memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding) * GLOBAL_RESOURCE_NUM);

      for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
      {
        VkDescriptorSetLayoutBinding& binding = bindings[i];
        binding.binding = i;
        binding.descriptorCount = RenderUtils::GetNumDescriptors(static_cast<GlobalResourceType>(i), someProperties);
        binding.descriptorType = RenderCore_PlatformVk::GetDescriptorType(static_cast<GlobalResourceType>(i));
        binding.stageFlags = VK_SHADER_STAGE_ALL;
      }

      myDescriptorSetLayout_GlobalResourcesSamplers = locCreateDescriptorSetLayout(bindings, GLOBAL_RESOURCE_NUM);
    }

    // Local buffer descriptor set
    {
      const uint numBindingsNeeded = someProperties.myNumLocalBuffers * 2 + someProperties.myNumLocalCBuffers;
      VkDescriptorSetLayoutBinding* bindings = (VkDescriptorSetLayoutBinding*)alloca(sizeof(VkDescriptorSetLayoutBinding) * numBindingsNeeded);
      memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding) * numBindingsNeeded);

      uint bindingIdx = 0;
      for (uint i = 0; i < someProperties.myNumLocalBuffers * 2; ++i)
      {
        VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
        binding.binding = bindingIdx;
        binding.descriptorCount = 1;
        // binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.stageFlags = VK_SHADER_STAGE_ALL;
        ++bindingIdx;
      }

      for (uint i = 0; i < someProperties.myNumLocalCBuffers; ++i)
      {
        VkDescriptorSetLayoutBinding& binding = bindings[bindingIdx];
        binding.binding = bindingIdx;
        binding.descriptorCount = 1;
        // binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.stageFlags = VK_SHADER_STAGE_ALL;
        ++bindingIdx;
      }

      ASSERT(bindingIdx == numBindingsNeeded);
      myDescriptorSetLayout_LocalBuffersCBuffers = locCreateDescriptorSetLayout(bindings, numBindingsNeeded);
    }

    // Create the pipeline layout
    VkDescriptorSetLayout descSetLayouts[] = { myDescriptorSetLayout_GlobalResourcesSamplers, myDescriptorSetLayout_LocalBuffersCBuffers };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pSetLayouts = descSetLayouts;
    pipelineLayoutInfo.setLayoutCount = ARRAY_LENGTH(descSetLayouts);

    VkDevice device = RenderCore::GetPlatformVk()->GetDevice();
    ASSERT_VK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &myPipelineLayout));
  }
//---------------------------------------------------------------------------//
  PipelineLayoutVk::~PipelineLayoutVk()
  {
    RenderCore::WaitForIdle(CommandListType::Graphics);
    RenderCore::WaitForIdle(CommandListType::Compute);

    VkDevice device = RenderCore::GetPlatformVk()->GetDevice();
    vkDestroyPipelineLayout(device, myPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, myDescriptorSetLayout_GlobalResourcesSamplers, nullptr);
    vkDestroyDescriptorSetLayout(device, myDescriptorSetLayout_LocalBuffersCBuffers, nullptr);
  }
//---------------------------------------------------------------------------//
}


#endif