#pragma once

#include "FileWatcher.h"
#include "ShaderCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderCompilerDX12 : public ShaderCompiler, public FileWatcher
  {
  public:
      ~ShaderCompilerDX12() override = default;

  protected:
    bool Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const override;
  };
//---------------------------------------------------------------------------//
}