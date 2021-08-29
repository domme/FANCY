#include "fancy_core_precompile.h"
#include "Shader.h"
#include "ShaderCompiler.h"

using namespace Fancy;

void Shader::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
{
  myProperties = aCompilerOutput.myProperties;
  myDesc = aCompilerOutput.myDesc;
  myVertexAttributes = aCompilerOutput.myVertexAttributes;
  myDefaultVertexInputLayout = aCompilerOutput.myDefaultVertexInputLayout;
}
