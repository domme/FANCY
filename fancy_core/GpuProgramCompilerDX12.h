#pragma once

#include "FileWatcher.h"
#include "GpuProgramCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerDX12 : public GpuProgramCompiler, public FileWatcher
  {
  public:
      ~GpuProgramCompilerDX12() override = default;

      bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutput* aCompilerOutput) const override;
      String ResolvePlatformShaderPath(const String& aPath) const override;
  };
//---------------------------------------------------------------------------//
}