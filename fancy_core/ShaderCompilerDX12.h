#pragma once

#include "FileWatcher.h"
#include "ShaderCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderCompilerDX12 : public ShaderCompiler, public FileWatcher
  {
  public:
      ~ShaderCompilerDX12() override = default;
      String GetShaderPath(const char* aPath) const override;

  protected:
    bool Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* aCompilerOutput) const override;
  };
//---------------------------------------------------------------------------//
}