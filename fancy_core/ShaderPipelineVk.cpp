#include "fancy_core_precompile.h"
#include "ShaderPipelineVk.h"
#include "ShaderVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderResourceInfoVk.h"
#include "PipelineLayoutCacheVk.h"
#include "PipelineLayoutVk.h"

#include "EASTL/sort.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  ShaderPipelineVk::ShaderPipelineVk()
    : myPipelineLayout(nullptr)
  {
  }

  ShaderPipelineVk::~ShaderPipelineVk()
  {
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
      const eastl::vector<ShaderResourceInfoVk>& shaderResourceInfos = shaderVk->GetResourceInfos();
      for (const ShaderResourceInfoVk& resourceInfo : shaderResourceInfos)
      {
        if (std::find(myResourceInfos.begin(), myResourceInfos.end(), resourceInfo) == myResourceInfos.end())
          myResourceInfos.push_back(resourceInfo);
      }
    }

    // Create or get an existing pipeline layout from the resource infos
    uint numSetsRequired = 0u;
    for (const ShaderResourceInfoVk& resourceInfo : myResourceInfos)
      numSetsRequired = glm::max(numSetsRequired, resourceInfo.myDescriptorSet + 1u);

    PipelineLayoutCreateInfoVk pipelineLayoutInfo;
    pipelineLayoutInfo.myDescriptorSetInfos.resize(numSetsRequired);

    for (const ShaderResourceInfoVk& resourceInfo : myResourceInfos)
    {
      eastl::fixed_vector<VkDescriptorSetLayoutBinding, 32>& targetBindingsInSet = pipelineLayoutInfo.myDescriptorSetInfos[resourceInfo.myDescriptorSet].myBindings;

#if FANCY_RENDERER_DEBUG
      for (int i = 0; i < (int)targetBindingsInSet.size(); ++i)
        ASSERT(targetBindingsInSet[i].binding != resourceInfo.myBindingInSet, "Binding %d is already used", resourceInfo.myBindingInSet);
#endif

      VkDescriptorSetLayoutBinding vkBinding;
      vkBinding.binding = resourceInfo.myBindingInSet;
      vkBinding.descriptorCount = resourceInfo.myNumDescriptors;
      vkBinding.descriptorType = resourceInfo.myType;
      vkBinding.stageFlags = VK_SHADER_STAGE_ALL;
      vkBinding.pImmutableSamplers = nullptr; // TODO: Needs to be used for static samplers in Vulkan shaders. Do that in the future! Otherwise we need to actually bind dynamic samplers
      targetBindingsInSet.push_back(vkBinding);
    }
 
    for (PipelineLayoutCreateInfoVk::DescriptorSetInfo& set : pipelineLayoutInfo.myDescriptorSetInfos)
    {
     eastl::stable_sort(set.myBindings.begin(), set.myBindings.begin() + set.myBindings.size() - 1, [](const VkDescriptorSetLayoutBinding& aLeft, const VkDescriptorSetLayoutBinding& aRight) {
        return aLeft.binding < aRight.binding;
     });
    }

    PipelineLayoutCacheVk& layoutCache = RenderCore::GetPlatformVk()->GetPipelineLayoutCache();
    myPipelineLayout = layoutCache.GetPipelineLayout(pipelineLayoutInfo);
  }
}

#endif
