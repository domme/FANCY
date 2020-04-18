#pragma once
#include "ShaderPipeline.h"
#include "VkPrerequisites.h"
#include "PipelineDescriptorSetLayoutsVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
  struct ShaderResourceInfoVk;

  class ShaderPipelineVk : public ShaderPipeline
  {
  public:
    ShaderPipelineVk();
    ~ShaderPipelineVk() override;

    void CreateFromShaders() override;

    const DynamicArray<ShaderResourceInfoVk>& GetResourceInfos() const { return myResourceInfos; }
    const PipelineDescriptorSetLayoutsVk& GetDescriptorSetLayouts() const { return myDescriptorSetLayouts; }
    VkDescriptorSetLayout GetDescriptorSetLayout(uint aSetIdx) const { return myDescriptorSetLayouts[aSetIdx]; }
    VkPipelineLayout GetPipelineLayout() const { return myPipelineLayout; }
    bool HasDescriptorSet(uint aSetIdx) const { return GetDescriptorSetLayout(aSetIdx) != nullptr; }

  private:
    VkPipelineLayout myPipelineLayout;
    PipelineDescriptorSetLayoutsVk myDescriptorSetLayouts;
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
  };
}

#endif