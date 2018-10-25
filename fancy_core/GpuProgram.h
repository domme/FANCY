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

    const GpuProgramDesc& GetDescription() const { return myDesc; }
    virtual void SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput);
    virtual uint64 GetNativeBytecodeHash() const = 0;
    
    GpuProgramDesc myDesc;

    ShaderResourceInterface* myResourceInterface;
    GpuProgramProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
