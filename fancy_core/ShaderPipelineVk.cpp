#include "fancy_core_precompile.h"
#include "ShaderPipelineVk.h"
#include "ShaderVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

namespace Fancy
{
  ShaderPipelineVk::~ShaderPipelineVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    if (myPipelineLayout != nullptr)
      vkDestroyPipelineLayout(platformVk->myDevice, myPipelineLayout, nullptr);
  }

  void ShaderPipelineVk::UpdateResourceInterface()
  {
    // Create the pipeline layout by analyzing the reflected data from all shaders

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

    if (myPipelineLayout != nullptr)
    {
      vkDestroyPipelineLayout(platformVk->myDevice, myPipelineLayout, nullptr);
      myPipelineLayout = nullptr;
    }

    struct DescriptorSet
    {
      DescriptorSet(uint aSetIndex) : mySet(aSetIndex) { }
      uint mySet;
      DynamicArray<VkDescriptorSetLayoutBinding> myBindings;
    };
    DynamicArray<DescriptorSet> descriptorSets;

    for (SharedPtr<Shader>& shader : myShaders)
    {
      if (shader == nullptr)
        continue;

      ShaderVk* shaderVk = static_cast<ShaderVk*>(shader.get());
      const ShaderBindingInfoVk& shaderBindingInfo = shaderVk->myBindingInfo;
      
      for (const ShaderDescriptorSetBindingInfoVk& setBindingInfo : shaderBindingInfo.myDescriptorSets)
      {
        int iSet = -1;
        for (int i = 0; i < (int)descriptorSets.size() && iSet == -1; ++i)
          if (descriptorSets[i].mySet == setBindingInfo.mySet)
            iSet = i;

        if (iSet == -1)
        {
          descriptorSets.push_back(DescriptorSet(setBindingInfo.mySet));
          iSet = (int) descriptorSets.size() - 1;
        }

        DynamicArray<VkDescriptorSetLayoutBinding>& targetBindingsInSet = descriptorSets[iSet].myBindings;
        for (const ShaderDescriptorBindingVk& bindingInfo : setBindingInfo.myBindings)
        {
          for (int i = 0; i < (int)targetBindingsInSet.size(); ++i)
            ASSERT(targetBindingsInSet[i].binding != bindingInfo.myBinding, "Binding %d is already used", bindingInfo.myBinding);

          VkDescriptorSetLayoutBinding vkBinding;
          vkBinding.binding = bindingInfo.myBinding;
          vkBinding.descriptorCount = bindingInfo.myDescriptorCount;
          vkBinding.descriptorType = bindingInfo.myDescriptorType;
          vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
          vkBinding.pImmutableSamplers = nullptr;  // TODO: Needs to be used for static samplers in Vulkan shaders. Do that in the future! Otherwise we need to actually bind dynamic samplers
          targetBindingsInSet.push_back(vkBinding);
        }
      }
    }

    // Create the VkDescriptorSetLayouts
    myDescriptorSetLayouts.resize(descriptorSets.size());
    for (uint i = 0u; i < (uint) descriptorSets.size(); ++i)
    {
      VkDescriptorSetLayoutCreateInfo createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.pNext = nullptr;
      createInfo.flags = 0u;
      createInfo.bindingCount = (uint) descriptorSets[i].myBindings.size();
      createInfo.pBindings = descriptorSets[i].myBindings.data();

      ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(platformVk->myDevice, &createInfo, nullptr, &myDescriptorSetLayouts[i]));
    }

    // Describe the pipeline layout using the descriptorSetLayouts
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.flags = 0u;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.pPushConstantRanges = nullptr;  // Don't use push constants for now
    layoutCreateInfo.pushConstantRangeCount = 0u;
    layoutCreateInfo.setLayoutCount = (uint)myDescriptorSetLayouts.size();
    layoutCreateInfo.pSetLayouts = myDescriptorSetLayouts.data();
    
    ASSERT_VK_RESULT(vkCreatePipelineLayout(platformVk->myDevice, &layoutCreateInfo, nullptr, &myPipelineLayout));
  }
}


