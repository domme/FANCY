#pragma once
#include "Shader.h"
#include "VkPrerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  struct ShaderDescriptorBindingVk
  {
    uint myBinding;
    VkDescriptorType myDescriptorType;
    uint myDescriptorCount;
  };
//---------------------------------------------------------------------------//
  struct ShaderDescriptorSetBindingInfoVk
  {
    uint mySet;
    DynamicArray<ShaderDescriptorBindingVk> myBindings;
  };
//---------------------------------------------------------------------------//
  struct ShaderBindingInfoVk
  {
    DynamicArray<ShaderDescriptorSetBindingInfoVk> myDescriptorSets;
  };
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataVk
  {
    VkShaderModule myModule = nullptr;
    ShaderBindingInfoVk myBindingInfo;
    DynamicArray<VkVertexInputAttributeDescription> myVertexAttributes;
  };
//---------------------------------------------------------------------------//
  class ShaderVk : public Shader
  {
    friend class ShaderCompilerVk;

  public:
    ~ShaderVk() override;

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;
    
    VkShaderModule myModule = nullptr;
    VkPipelineShaderStageCreateInfo myShaderStageCreateInfo = {};
    VkPipelineVertexInputStateCreateInfo myVertexInputCreateInfo = {};
    ShaderBindingInfoVk myBindingInfo;
    DynamicArray<VkVertexInputAttributeDescription> myVertexAttributes;
  };
//---------------------------------------------------------------------------//
}
