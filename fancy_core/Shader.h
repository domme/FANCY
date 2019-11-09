#pragma once

#include "FancyCoreDefines.h"
#include "ShaderDesc.h"
#include "VertexInputLayout.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  struct ShaderProperties
  {
    ShaderVertexInputLayout myVertexInputLayout;  // TODO: Remove and replace with a platform-specific version. We don't need that in high-level code
    ShaderStage myShaderStage = ShaderStage::NONE;
    bool myHasUnorderedWrites = false;
    glm::int3 myNumGroupThreads = { 1, 1, 1 };
  };
//---------------------------------------------------------------------------//
  class Shader
  {
  public:
    virtual ~Shader() = default;

    const ShaderDesc& GetDescription() const { return myDesc; }
    virtual void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput);
    virtual uint64 GetNativeBytecodeHash() const = 0;
    
    ShaderDesc myDesc;
    ShaderProperties myProperties;
  };
//---------------------------------------------------------------------------//
}
