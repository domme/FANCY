#pragma once

#include "ShaderDesc.h"
#include "Shader.h"
#include "DxcShaderCompiler.h"

#if FANCY_ENABLE_DX12
  #include "DX12/ShaderDX12.h"
#endif

namespace Fancy {
  struct RtShaderCompiledData;
  struct RtShaderProperties;
  //---------------------------------------------------------------------------//
  struct ShaderDesc;
//---------------------------------------------------------------------------//
  struct ShaderCompilerResult
  { 
    ShaderDesc myDesc;
    ShaderProperties myProperties;
    eastl::fixed_vector<VertexShaderAttributeDesc, 16> myVertexAttributes;
    eastl::fixed_vector<eastl::string, 16> myIncludedFilePaths;
    SharedPtr<VertexInputLayout> myDefaultVertexInputLayout;

#if FANCY_ENABLE_DX12
    ShaderCompiledDataDX12 myDx12Data;
#endif
  };
//---------------------------------------------------------------------------//
  class ShaderCompiler
  {
    public:
      static const char* ShaderStageToDefineString(ShaderStage aShaderStage);
      static FixedShortString GetHLSLprofileString(ShaderStage aShaderStage, ShaderModel aShaderModel = SM_LATEST);
      static VertexAttributeSemantic GetVertexAttributeSemantic(const char* aSemanticName);

      virtual ~ShaderCompiler() = default;

      bool Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const;

  protected:
      virtual bool Compile_Internal(const char* anHlslSrcPathAbs, const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const = 0;

      DxcShaderCompiler myDxcCompiler;
  };
//---------------------------------------------------------------------------//
}
