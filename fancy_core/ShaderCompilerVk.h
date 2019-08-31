#pragma once
#include "ShaderCompiler.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  class ShaderCompilerVk : public ShaderCompiler
  {
  public:
    ShaderCompilerVk();
    virtual ~ShaderCompilerVk();

    String GetShaderPath(const String& aFilename) const override;

  protected:
    bool Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* aCompilerOutput) const override;
  };
//---------------------------------------------------------------------------//
}


