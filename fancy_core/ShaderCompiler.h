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
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
  class ShaderCompiler
  {
    public:
      static const char* GetShaderRootFolderRelative() { return "shader"; }
      static const char* ShaderStageToDefineString(ShaderStage aShaderStage);
      static const char* GetHLSLprofileString(ShaderStage aShaderStage, ShaderModel aShaderModel = ShaderModel::SM_6_1);

      virtual ~ShaderCompiler() = default;
      virtual String GetShaderPath(const char* aShaderName) const;

      bool Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const;

  protected:
      virtual bool Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const = 0;

      DxcShaderCompiler myDxcCompiler;
  };
//---------------------------------------------------------------------------//
}
