#pragma once
#include "GpuProgramCompiler.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class GpuProgramCompilerVk : public GpuProgramCompiler
  {
  public:
    GpuProgramCompilerVk();
    virtual ~GpuProgramCompilerVk();

    bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutput* aCompilerOutput) const override;
    String ResolvePlatformShaderPath(const String& aPath) const override;
  };
//---------------------------------------------------------------------------//
}


