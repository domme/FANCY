#include "GpuProgram.h"
#include "GpuProgramCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GpuProgram::GpuProgram()
    : myStage(ShaderStage::NONE)
    , myResourceInterface(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  GpuProgramDesc GpuProgram::GetDescription() const
  {
    GpuProgramDesc desc;
    desc.myShaderFileName = mySourcePath;
    desc.myShaderStage = static_cast<uint>(myStage);
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
      myStage = static_cast<ShaderStage>(aDesc.myShaderStage);

      SetFromCompilerOutput(output);
    }

    return success;
  }
  //---------------------------------------------------------------------------//
  void GpuProgram::SetFromCompilerOutput(const GpuProgramCompilerOutput& aCompilerOutput)
  {
    myReadTextureInfos = aCompilerOutput.vReadTextureInfos;
    myReadBufferInfos = aCompilerOutput.vReadBufferInfos;
    myWriteTextureInfos = aCompilerOutput.vWriteTextureInfos;
    myWriteBufferInfos = aCompilerOutput.vWriteBufferInfos;
    myConstantBufferElements = aCompilerOutput.myConstantBufferElements;
    myStage = aCompilerOutput.eShaderStage;
    mySourcePath = aCompilerOutput.myShaderFilename;
    myPermutation = aCompilerOutput.myPermutation;
    myInputLayout = aCompilerOutput.clVertexInputLayout;
    myResourceInterface = aCompilerOutput.myRootSignature;
  }
//---------------------------------------------------------------------------//
}
