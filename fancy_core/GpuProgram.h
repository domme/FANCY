#pragma once

#include "FancyCorePrerequisites.h"
#include "GpuProgramFeatures.h"
#include "GpuProgramDesc.h"
#include "GpuProgramProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutput;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  class GpuProgram
  {
  public:

    GpuProgram();
    virtual ~GpuProgram() = default;

    GpuProgramDesc GetDescription() const;
    virtual void SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput);
    virtual uint64 GetNativeBytecodeHash() const = 0;
    
    String mySourcePath;
    GpuProgramPermutation myPermutation;
    ShaderResourceInterface* myResourceInterface;
    GpuProgramProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
