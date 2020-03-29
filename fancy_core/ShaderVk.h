#pragma once
#include "Shader.h"
#include "VkPrerequisites.h"
#include "ShaderResourceInfoVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  struct ShaderVertexAttributeDescVk
  {
    DynamicArray<VkVertexInputAttributeDescription> myVertexAttributes;
    uint myOverallVertexSize;
  };
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataVk
  {
    VkShaderModule myModule = nullptr;
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
    ShaderVertexAttributeDescVk myVertexAttributeDesc;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
    class ShaderVk : public Shader
  {
    friend class ShaderCompilerVk;

  public:
    ~ShaderVk() override;

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;

    const DynamicArray<ShaderResourceInfoVk>& GetResourceInfos() const { return myResourceInfos; }
    
    VkShaderModule myModule = nullptr;
    VkPipelineShaderStageCreateInfo myShaderStageCreateInfo = {};
    VkPipelineVertexInputStateCreateInfo myVertexInputCreateInfo = {};
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
    ShaderVertexAttributeDescVk myVertexAttributeDesc;
  };
//---------------------------------------------------------------------------//
}

#endif