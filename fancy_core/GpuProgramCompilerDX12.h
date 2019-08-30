#pragma once

#include "FileWatcher.h"
#include "GpuProgramCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerDX12 : public GpuProgramCompiler, public FileWatcher
  {
  public:
      ~GpuProgramCompilerDX12() override = default;
      String ResolvePlatformShaderPath(const String& aPath) const override;

  protected:
    bool Compile_Internal(const GpuProgramDesc& aDesc, const char** someDefines, uint aNumDefines, GpuProgramCompilerOutput* aCompilerOutput) const override;
  };
//---------------------------------------------------------------------------//
}