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
  struct ShaderCompiledDataVk
  {
    VkShaderModule myModule = nullptr;
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
    StaticArray<uint, 16> myVertexAttributeLocations;
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
    StaticArray<uint, 16> myVertexAttributeLocations;
    DynamicArray<ShaderResourceInfoVk> myResourceInfos;
    uint64 myBytecodeHash = 0ull;
  };
//---------------------------------------------------------------------------//
}

#endif