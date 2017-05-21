#pragma once

#include "FancyCorePrerequisites.h"
#include "FileWatcher.h"
#include "GpuProgramCompiler.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerDX12 : public GpuProgramCompiler, public FileWatcher
  {
  public:
      ~GpuProgramCompilerDX12() override = default;

      bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutput* aCompilerOutput) const override;
      String ResolvePlatformShaderPath(const String& aPath) const override;
  };
//---------------------------------------------------------------------------//
} } }

