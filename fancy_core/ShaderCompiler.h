#pragma once

#include "ShaderDesc.h"
#include "Shader.h"
#include "DxcShaderCompiler.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderDesc;
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult
  { 
    ShaderDesc myDesc;
    ShaderProperties myProperties;
    StaticArray<VertexShaderAttributeDesc, 16> myVertexAttributes;
    SharedPtr<VertexInputLayout> myDefaultVertexInputLayout;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
  class ShaderCompiler
  {
    public:
      static const char* GetShaderRootFolderRelative() { return "resources/shader"; }
      static const char* ShaderStageToDefineString(ShaderStage aShaderStage);
      static const char* GetHLSLprofileString(ShaderStage aShaderStage, ShaderModel aShaderModel = ShaderModel::SM_6_1);
      static VertexAttributeSemantic GetVertexAttributeSemantic(const char* aSemanticName);

      virtual ~ShaderCompiler() = default;
      
      String GetShaderPathRelative(const char* aRelativeShaderPath) const;

      bool Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const;

  protected:
      virtual bool Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const = 0;

      DxcShaderCompiler myDxcCompiler;
  };
//---------------------------------------------------------------------------//
}
