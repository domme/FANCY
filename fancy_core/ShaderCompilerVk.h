#pragma once
#include "ShaderCompiler.h"

struct IDxcLibrary;
struct IDxcCompiler;

namespace Fancy
{
//---------------------------------------------------------------------------//
  class ShaderCompilerVk : public ShaderCompiler
  {
  public:
    ShaderCompilerVk();
    virtual ~ShaderCompilerVk();

  protected:
    bool Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const override;
  };
//---------------------------------------------------------------------------//
}


