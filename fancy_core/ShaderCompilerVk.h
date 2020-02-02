#pragma once
#include "ShaderCompiler.h"

class IDxcLibrary;
class IDxcCompiler;

namespace Fancy
{
//---------------------------------------------------------------------------//
  class ShaderCompilerVk : public ShaderCompiler
  {
  public:
    ShaderCompilerVk();
    virtual ~ShaderCompilerVk();

    String GetShaderPath(const char* aFilename) const override;

  protected:
    bool Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* aCompilerOutput) const override;

    Microsoft::WRL::ComPtr<IDxcLibrary> myDxcLibrary;
    Microsoft::WRL::ComPtr<IDxcCompiler> myDxcCompiler;
  };
//---------------------------------------------------------------------------//
}


