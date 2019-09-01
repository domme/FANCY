#pragma once
#include "Shader.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
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
