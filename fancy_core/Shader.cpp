#include "fancy_core_precompile.h"
#include "Shader.h"
#include "ShaderCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Shader::Shader()
    : myResourceInterface(nullptr)
  {
  }
//---------------------------------------------------------------------------//
  void Shader::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    myProperties = aCompilerOutput.myProperties;
    myResourceInterface = aCompilerOutput.myRootSignature;
    myDesc = aCompilerOutput.myDesc;
  }
//---------------------------------------------------------------------------//
}
