#pragma once
#include "Shader.h"
#include "VkPrerequisites.h"
#include "ShaderResourceInfoVk.h"
#include "eastl/vector.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataVk
  {
    VkShaderModule myModule = nullptr;
    eastl::vector<ShaderResourceInfoVk> myResourceInfos;
    eastl::fixed_vector<uint, 16> myVertexAttributeLocations;
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

    const eastl::vector<ShaderResourceInfoVk>& GetResourceInfos() const { return myResourceInfos; }
    VkShaderModule GetModule() const { return myModule; }
    const VkPipelineShaderStageCreateInfo& GetStageCreateInfo() const { return myShaderStageCreateInfo; }
    
    VkShaderModule myModule = nullptr;
    VkPipelineShaderStageCreateInfo myShaderStageCreateInfo = {};
    eastl::fixed_vector<uint, 16> myVertexAttributeLocations;
    eastl::vector<ShaderResourceInfoVk> myResourceInfos;
    uint64 myBytecodeHash = 0ull;
  };
//---------------------------------------------------------------------------//
}

#endif