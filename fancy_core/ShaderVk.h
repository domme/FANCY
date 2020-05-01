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
    uint64 myBytecodeHash = 0ull;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
    class ShaderVk : public Shader
  {
    friend class ShaderCompilerVk;

  public:
    ~ShaderVk() override;

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override { return myBytecodeHash; }

    const DynamicArray<ShaderResourceInfoVk>& GetResourceInfos() const { return myResourceInfos; }
    VkShaderModule GetModule() const { return myModule; }
    const VkPipelineShaderStageCreateInfo& GetStageCreateInfo() const { return myShaderStageCreateInfo; }
    
    VkShaderModule myModule = nullptr;
    VkPipelineShaderStageCreateInfo myShaderStageCreateInfo = {};
    VkPipelineVertexInputStateCreateInfo myVertexInputCreateInfo = {};
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
    ShaderVertexAttributeDescVk myVertexAttributeDesc;
    uint64 myBytecodeHash = 0ull;
  };
//---------------------------------------------------------------------------//
}

#endif