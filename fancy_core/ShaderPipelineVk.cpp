#include "fancy_core_precompile.h"
#include "ShaderPipelineVk.h"
#include "ShaderVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderResourceInfoVk.h"

#if FANCY_ENABLE_VK

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

    myResourceInfos.clear();

    // Loop through all shaders and find all unique resources. Create both a descriptorSet-structure to create descriptorset layouts and also merge all resource infos into the pipeline
    for (SharedPtr<Shader>& shader : myShaders)
    {
      if (shader == nullptr)
        continue;

      ShaderVk* shaderVk = static_cast<ShaderVk*>(shader.get());
      const DynamicArray<ShaderResourceInfoVk>& shaderResourceInfos = shaderVk->GetResourceInfos();

      for (const ShaderResourceInfoVk& resourceInfo : shaderResourceInfos)
      {
        if (std::find(myResourceInfos.begin(), myResourceInfos.end(), resourceInfo) == myResourceInfos.end())
          myResourceInfos.push_back(resourceInfo);

        int iSet = -1;
        for (int i = 0; i < (int)descriptorSets.size() && iSet == -1; ++i)
          if (descriptorSets[i].mySet == resourceInfo.myDescriptorSet)
            iSet = i;

        if (iSet == -1)
        {
          descriptorSets.push_back(DescriptorSet(resourceInfo.myDescriptorSet));
          iSet = (int)descriptorSets.size() - 1;
        }

        DynamicArray<VkDescriptorSetLayoutBinding>& targetBindingsInSet = descriptorSets[iSet].myBindings;

        for (int i = 0; i < (int)targetBindingsInSet.size(); ++i)
          ASSERT(targetBindingsInSet[i].binding != resourceInfo.myBindingInSet, "Binding %d is already used", resourceInfo.myBindingInSet);

        VkDescriptorSetLayoutBinding vkBinding;
        vkBinding.binding = resourceInfo.myBindingInSet;
        vkBinding.descriptorCount = resourceInfo.myNumDescriptors;
        vkBinding.descriptorType = resourceInfo.myType;
        vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
        vkBinding.pImmutableSamplers = nullptr;  // TODO: Needs to be used for static samplers in Vulkan shaders. Do that in the future! Otherwise we need to actually bind dynamic samplers
        targetBindingsInSet.push_back(vkBinding);
      }
    }

    uint maxSetIdx = 0u;
    for (uint i = 0u; i < (uint)descriptorSets.size(); ++i)
      maxSetIdx = glm::max(maxSetIdx, descriptorSets[i].mySet);

    myDescriptorSetLayouts.GrowToSize(maxSetIdx + 1);
    for (uint i = 0u; i < myDescriptorSetLayouts.Size(); ++i)
      myDescriptorSetLayouts[i] = nullptr;

    // Create the VkDescriptorSetLayouts
    StaticArray<VkDescriptorSetLayout, 32> createdLayouts;
    for (uint i = 0u; i < (uint) descriptorSets.size(); ++i)
    {
      VkDescriptorSetLayoutCreateInfo createInfo;
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.pNext = nullptr;
      createInfo.flags = 0u;
      createInfo.bindingCount = (uint) descriptorSets[i].myBindings.size();
      createInfo.pBindings = descriptorSets[i].myBindings.data();

      ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(platformVk->myDevice, &createInfo, nullptr, &myDescriptorSetLayouts[descriptorSets[i].mySet]));
      createdLayouts.Add(myDescriptorSetLayouts[descriptorSets[i].mySet]);
    }

    // Describe the pipeline layout using the descriptorSetLayouts
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.flags = 0u;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.pPushConstantRanges = nullptr;  // Don't use push constants for now
    layoutCreateInfo.pushConstantRangeCount = 0u;
    layoutCreateInfo.setLayoutCount = (uint)createdLayouts.Size();
    layoutCreateInfo.pSetLayouts = createdLayouts.GetBuffer();
    
    ASSERT_VK_RESULT(vkCreatePipelineLayout(platformVk->myDevice, &layoutCreateInfo, nullptr, &myPipelineLayout));
  }
}

#endif  