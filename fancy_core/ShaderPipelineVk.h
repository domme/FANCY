#pragma once
#include "ShaderPipeline.h"
#include "VkPrerequisites.h"

namespace Fancy
{
  struct ShaderResourceInfoVk;

  class ShaderPipelineVk : public ShaderPipeline
  {
  public:
    ~ShaderPipelineVk() override;

    void UpdateResourceInterface() override;

    const DynamicArray<ShaderResourceInfoVk>& GetResourceInfos() const { return myResourceInfos; }
    VkDescriptorSetLayout GetDescriptorSetLayout(uint aSetIdx) const { return myDescriptorSetLayouts[aSetIdx]; }

    VkPipelineLayout myPipelineLayout;
    StaticArray<VkDescriptorSetLayout, 32> myDescriptorSetLayouts;
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
  };
}



