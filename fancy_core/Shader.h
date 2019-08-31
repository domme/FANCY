#pragma once

#include "FancyCoreDefines.h"
#include "ShaderDesc.h"
#include "ShaderProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  class Shader
  {
  public:

    Shader();
    virtual ~Shader() = default;

    const ShaderDesc& GetDescription() const { return myDesc; }
    virtual void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput);
    virtual uint64 GetNativeBytecodeHash() const = 0;
    
    ShaderDesc myDesc;

    ShaderResourceInterface* myResourceInterface;
    ShaderProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
