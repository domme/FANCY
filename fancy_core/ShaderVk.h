#pragma once
#include "Shader.h"
#include "VkPrerequisites.h"
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
    uint64 GetNativeBytecodeHash() const override { return myCompiledData.myBytecodeHash; }

    VkShaderModule GetModule() const { return myCompiledData.myModule; }
    const VkPipelineShaderStageCreateInfo& GetStageCreateInfo() const { return myShaderStageCreateInfo; }

    ShaderCompiledDataVk myCompiledData;
    VkPipelineShaderStageCreateInfo myShaderStageCreateInfo = {};
  };
//---------------------------------------------------------------------------//
}

#endif