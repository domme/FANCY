#pragma once
#include "Shader.h"
#include "VkPrerequisites.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  struct ShaderCompiledDataVk
  {
    VkShaderModule myModule = nullptr;
  };
//---------------------------------------------------------------------------//
  class ShaderVk : public Shader
  {
    friend class ShaderCompilerVk;

  public:
    ShaderVk();
    ~ShaderVk() override;

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;
  };
//---------------------------------------------------------------------------//
}
