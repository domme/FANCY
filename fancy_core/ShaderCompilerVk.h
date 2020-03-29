#pragma once
#include "ShaderCompiler.h"

#if FANCY_ENABLE_VK

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

#endif


