#include "fancy_core_precompile.h"
#include "ShaderCompiler.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  const char* ShaderCompiler::ShaderStageToDefineString(ShaderStage aShaderStage)
  {
    switch (aShaderStage)
    {
    case Fancy::ShaderStage::VERTEX:
      return "PROGRAM_TYPE_VERTEX";
    case Fancy::ShaderStage::FRAGMENT:
      return "PROGRAM_TYPE_FRAGMENT";
    case Fancy::ShaderStage::GEOMETRY:
      return "PROGRAM_TYPE_GEOMETRY";
    case Fancy::ShaderStage::TESS_HULL:
      return "PROGRAM_TYPE_TESS_HULL";
    case Fancy::ShaderStage::TESS_DOMAIN:
      return "PROGRAM_TYPE_TESS_DOMAIN";
    case Fancy::ShaderStage::COMPUTE:
      return "PROGRAM_TYPE_COMPUTE";
    default:
      return "";
    }
  }
//---------------------------------------------------------------------------//
  const char* ShaderCompiler::GetHLSLprofileString(ShaderStage aShaderStage, ShaderModel aShaderModel)
  {
    switch (aShaderModel)
    {
      case ShaderModel::SM_5_1: 
      {
        switch (aShaderStage)
        {
          case ShaderStage::VERTEX: return "vs_5_1";
          case ShaderStage::FRAGMENT: return "ps_5_1";
          case ShaderStage::GEOMETRY: return "gs_5_1";
          case ShaderStage::COMPUTE: return "cs_5_1";
          default:
            ASSERT(false, "Unsupported HLSL shader-profile");
            return "";
        }
      }
      default: ASSERT(false, "Unsupported shader model");
        return "";
    }
  }
//---------------------------------------------------------------------------//
  bool ShaderCompiler::Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const
  {
    LOG_INFO("Compiling shader %s...", aDesc.myShaderFileName.c_str());

    const char* stageDefine = ShaderStageToDefineString(static_cast<ShaderStage>(aDesc.myShaderStage));
    const bool success = Compile_Internal(aDesc, stageDefine, aCompilerOutput);
    if (success)
    {
      aCompilerOutput->myDesc = aDesc;
      aCompilerOutput->myProperties.myShaderStage = static_cast<ShaderStage>(aDesc.myShaderStage);
    }
    return success;
  }
 //---------------------------------------------------------------------------//
}
