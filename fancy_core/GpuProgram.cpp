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
  bool GpuProgram::SetFromDescription(const GpuProgramDesc& aDesc, const GpuProgramCompiler* aCompiler)
  {
    GpuProgramCompilerOutput output;
    const bool success = aCompiler->Compile(aDesc, &output);

    if (success)
    {
      mySourcePath = aDesc.myShaderFileName;
      myPermutation = aDesc.myPermutation;

      SetFromCompilerOutput(output);
    }

    return success;
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
