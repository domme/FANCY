#include "GpuProgram.h"
#include "GpuProgramCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuProgram::GpuProgram()
    : myResourceInterface(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDesc GpuProgram::GetDescription() const
  {
    GpuProgramDesc desc;
    desc.myShaderFileName = mySourcePath;
    desc.myShaderStage = (uint) myProperties.myShaderStage;
    desc.myPermutation = myPermutation;
    return desc;
  }
//---------------------------------------------------------------------------//
  void GpuProgram::SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput)
  {
    myProperties = aCompilerOutput.myProperties;
    mySourcePath = aCompilerOutput.myShaderFilename;
    myPermutation = aCompilerOutput.myPermutation;
    myResourceInterface = aCompilerOutput.myRootSignature;
  }
//---------------------------------------------------------------------------//
}
