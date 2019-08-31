#pragma once
#include "Shader.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  class GpuProgramVk : public Shader
  {
    friend class ShaderCompilerVk;

  public:
    GpuProgramVk();
    virtual ~GpuProgramVk() override;

    void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput) override;
    uint64 GetNativeBytecodeHash() const override;
  };
//---------------------------------------------------------------------------//
}
