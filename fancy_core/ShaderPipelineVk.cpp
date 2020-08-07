#include "fancy_core_precompile.h"
#include "ShaderPipelineVk.h"
#include "ShaderVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderResourceInfoVk.h"
#include "PipelineLayoutCacheVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  ShaderPipelineVk::ShaderPipelineVk()
    : myPipelineLayout(nullptr)
  {
  }

  ShaderPipelineVk::~ShaderPipelineVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    if (myPipelineLayout != nullptr)
      vkDestroyPipelineLayout(platformVk->myDevice, myPipelineLayout, nullptr);
  }

  void ShaderPipelineVk::CreateFromShaders()
  {
#if FANCY_RENDERER_DEBUG
    const bool hasComputeShader = myShaders[(uint)ShaderStage::COMPUTE] != nullptr;
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      if (myShaders[i] == nullptr)
        continue;

      if (hasComputeShader)
      {
        ASSERT(i == (uint)ShaderStage::COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");
      }
      else
      {
        ASSERT(i != (uint)ShaderStage::COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");
      }
    }
#endif

    // Merge Resource infos
    myResourceInfos.clear();
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      Shader* shader = myShaders[i].get();
      if (shader == nullptr)
        continue;

      ShaderVk* shaderVk = static_cast<ShaderVk*>(shader);
      const DynamicArray<ShaderResourceInfoVk>& shaderResourceInfos = shaderVk->GetResourceInfos();
      for (const ShaderResourceInfoVk& resourceInfo : shaderResourceInfos)
      {
        if (std::find(myResourceInfos.begin(), myResourceInfos.end(), resourceInfo) == myResourceInfos.end())
          myResourceInfos.push_back(resourceInfo);
      }
    }

    // Create descriptor set layouts from the resource infos
    DynamicArray<PipelineLayoutCacheVk::DescriptorSetInfo> descriptorSets;
    uint maxSetIdx = 0u;
    for (const ShaderResourceInfoVk& resourceInfo : myResourceInfos)
    {
      int iSet = -1;
      for (int i = 0; i < (int)descriptorSets.size() && iSet == -1; ++i)
        if (descriptorSets[i].mySet == resourceInfo.myDescriptorSet)
          iSet = i;

      if (iSet == -1)
      {
        descriptorSets.push_back(PipelineLayoutCacheVk::DescriptorSetInfo(resourceInfo.myDescriptorSet));
        iSet = (int)descriptorSets.size() - 1;
      }

      DynamicArray<VkDescriptorSetLayoutBinding>& targetBindingsInSet = descriptorSets[iSet].myBindings;
      maxSetIdx = glm::max(maxSetIdx, descriptorSets[iSet].mySet);

#if FANCY_RENDERER_DEBUG
      for (int i = 0; i < (int)targetBindingsInSet.size(); ++i)
      ASSERT(targetBindingsInSet[i].binding != resourceInfo.myBindingInSet, "Binding %d is already used", resourceInfo.
myBindingInSet);
#endif

      VkDescriptorSetLayoutBinding vkBinding;
      vkBinding.binding = resourceInfo.myBindingInSet;
      vkBinding.descriptorCount = resourceInfo.myNumDescriptors;
      vkBinding.descriptorType = resourceInfo.myType;
      vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
      vkBinding.pImmutableSamplers = nullptr;
      // TODO: Needs to be used for static samplers in Vulkan shaders. Do that in the future! Otherwise we need to actually bind dynamic samplers
      targetBindingsInSet.push_back(vkBinding);
    }

    if (!descriptorSets.empty())
    {
      std::stable_sort(descriptorSets.begin(), descriptorSets.begin() + descriptorSets.size() - 1, [](const PipelineLayoutCacheVk::DescriptorSetInfo& aLeft, const PipelineLayoutCacheVk::DescriptorSetInfo& aRight) {
        return aLeft.mySet < aRight.mySet;
      });
    }
 
    for (uint i = 0u; i < (uint)descriptorSets.size(); ++i)
    {
     PipelineLayoutCacheVk::DescriptorSetInfo& descriptorSet = descriptorSets[i];
     std::stable_sort(descriptorSet.myBindings.begin(), descriptorSet.myBindings.begin() + descriptorSet.myBindings.size() - 1, [](const VkDescriptorSetLayoutBinding& aLeft, const VkDescriptorSetLayoutBinding& aRight) {
        return aLeft.binding < aRight.binding;
     });
    }

    myDescriptorSetLayouts = PipelineDescriptorSetLayoutsVk();

    PipelineLayoutCacheVk& layoutCache = RenderCore::GetPlatformVk()->GetPipelineLayoutCache();
    myPipelineLayout = layoutCache.GetPipelineLayout(descriptorSets, myDescriptorSetLayouts);
  }
}

#endif
