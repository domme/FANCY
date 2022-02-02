#include "fancy_core_precompile.h"

#if FANCY_ENABLE_VK

#include "Rendering/RenderCore.h"

#include "PipelineLayoutVk.h"
#include "RenderCore_PlatformVk.h"

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
    // Local buffers
    {
      // Might be _DYNAMIC in the future
      myDescriptorType_LocalBuffers = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      myDescriptorType_LocalRwBuffers = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

      eastl::vector<VkDescriptorSetLayoutBinding> bindings(someProperties.myNumLocalBuffers);
      for (uint i = 0; i < someProperties.myNumLocalBuffers; ++i)
      {
        VkDescriptorSetLayoutBinding& binding = bindings[i];
        binding.binding = i;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.stageFlags = VK_SHADER_STAGE_ALL;
      }

      myDescriptorSetLayout_LocalBuffers = locCreateDescriptorSetLayout(bindings.data(), static_cast<uint>(bindings.size()));
      myDescriptorSetLayout_LocalRwBuffers = locCreateDescriptorSetLayout(bindings.data(), static_cast<uint>(bindings.size()));
    }

    // Local Cbuffers
    {
      // Might be _DYNAMIC in the future
      myDescriptorType_LocalCBuffers = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

      eastl::vector<VkDescriptorSetLayoutBinding> bindings(someProperties.myNumLocalCBuffers);
      for (uint i = 0; i < someProperties.myNumLocalCBuffers; ++i)
      {
        VkDescriptorSetLayoutBinding& binding = bindings[i];
        binding.binding = i;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.stageFlags = VK_SHADER_STAGE_ALL;
      }

      myDescriptorSetLayout_LocalCbuffers = locCreateDescriptorSetLayout(bindings.data(), static_cast<uint>(bindings.size()));
    }

    // Resource- & Sampler-descriptor set
    {
      eastl::vector<VkDescriptorSetLayoutBinding> bindings(GLOBAL_RESOURCE_NUM);

      for (uint i = 0; i < GLOBAL_RESOURCE_NUM; ++i)
      {
        VkDescriptorSetLayoutBinding& binding = bindings[i];
        binding.binding = i;
        binding.descriptorCount = RenderCore::GetNumDescriptors(static_cast<GlobalResourceType>(i), someProperties);
        binding.descriptorType = RenderCore_PlatformVk::GetDescriptorType(static_cast<GlobalResourceType>(i));
        binding.stageFlags = VK_SHADER_STAGE_ALL;
      }

      myDescriptorSetLayout_GlobalResourcesSamplers = locCreateDescriptorSetLayout(bindings.data(), static_cast<uint>(bindings.size()));
    }
      
    // Create the pipeline layout
    VkDescriptorSetLayout descSetLayouts[] = {
      myDescriptorSetLayout_LocalBuffers,
      myDescriptorSetLayout_LocalRwBuffers,
      myDescriptorSetLayout_LocalCbuffers,
      myDescriptorSetLayout_GlobalResourcesSamplers };

    myDescriptorSetIndex_LocalBuffers = 0;
    myDescriptorSetIndex_LocalRwBuffers = 1;
    myDescriptorSetIndex_LocalCbuffers = 2;
    myDescriptorSetIndex_GlobalResourcesSamplers = 3;

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

    const VkDevice device = RenderCore::GetPlatformVk()->GetDevice();
    vkDestroyPipelineLayout(device, myPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, myDescriptorSetLayout_LocalBuffers, nullptr);
    vkDestroyDescriptorSetLayout(device, myDescriptorSetLayout_LocalRwBuffers, nullptr);
    vkDestroyDescriptorSetLayout(device, myDescriptorSetLayout_LocalCbuffers, nullptr);
    vkDestroyDescriptorSetLayout(device, myDescriptorSetLayout_GlobalResourcesSamplers, nullptr);
  }
//---------------------------------------------------------------------------//
}


#endif