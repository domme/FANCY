#include "GpuProgram.h"
#include "GpuProgramCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuProgram::GpuProgram()
    : myResourceInterface(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  void GpuProgram::SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput)
  {
    myProperties = aCompilerOutput.myProperties;
    myResourceInterface = aCompilerOutput.myRootSignature;
    myDesc = aCompilerOutput.myDesc;
  }
//---------------------------------------------------------------------------//
}
