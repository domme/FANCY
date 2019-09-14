#pragma once

#include "ShaderProperties.h"
#include "ShaderDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderDesc;
  class ShaderResourceInterface;
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult
  { 
    ShaderResourceInterface* myRootSignature = nullptr;
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
      static const char* GetHLSLprofileString(ShaderStage aShaderStage, ShaderModel aShaderModel = ShaderModel::SM_5_1);

      virtual ~ShaderCompiler() = default;
      virtual String GetShaderPath(const char* aShaderName) const = 0;

      bool Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const;

  protected:
    virtual bool Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* aCompilerOutput) const = 0;

  };
//---------------------------------------------------------------------------//
}
