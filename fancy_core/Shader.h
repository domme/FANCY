#pragma once

#include "FancyCoreDefines.h"
#include "ShaderDesc.h"
#include "VertexInputLayoutProperties.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult;
//---------------------------------------------------------------------------//
  struct VertexShaderAttributeDesc
  {
    VertexAttributeSemantic mySemantic;
    uint mySemanticIndex;
    DataFormat myFormat;
  };
//---------------------------------------------------------------------------//
  struct ShaderProperties
  {
    ShaderStage myShaderStage = SHADERSTAGE_NONE;
    glm::int3 myNumGroupThreads = { 1, 1, 1 };
  };
//---------------------------------------------------------------------------//
  class Shader
  {
  public:
    virtual ~Shader() = default;

    const ShaderDesc& GetDescription() const { return myDesc; }
    const ShaderProperties& GetProperties() const { return myProperties; }

    virtual void SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput);
    virtual uint64 GetNativeBytecodeHash() const = 0;
    
    ShaderDesc myDesc;
    ShaderProperties myProperties;
    eastl::fixed_vector<VertexShaderAttributeDesc, 16> myVertexAttributes;
    SharedPtr<VertexInputLayout> myDefaultVertexInputLayout;
  };
//---------------------------------------------------------------------------//
}
