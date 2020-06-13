#include "fancy_core_precompile.h"
#include "Shader.h"
#include "ShaderCompiler.h"
#include "MathUtil.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  uint64 Shader::GetParameterNameHash(const char* aName)
  {
    return MathUtil::Hash(aName);
  }
//---------------------------------------------------------------------------//
  void Shader::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    myProperties = aCompilerOutput.myProperties;
    myDesc = aCompilerOutput.myDesc;
    myVertexAttributes = aCompilerOutput.myVertexAttributes;
    myDefaultVertexInputLayout = aCompilerOutput.myDefaultVertexInputLayout;
  }
//---------------------------------------------------------------------------//
}
