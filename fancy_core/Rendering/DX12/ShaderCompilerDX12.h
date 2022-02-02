#pragma once

#include "IO/FileWatcher.h"
#include "Rendering/ShaderCompiler.h"

#if FANCY_ENABLE_DX12

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

#endif
