#include "fancy_core_precompile.h"
#include "ShaderCompiler.h"
#include "PathService.h"

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
      case ShaderModel::SM_6_0:
      {
        switch (aShaderStage)
        {
        case ShaderStage::VERTEX: return "vs_6_0";
        case ShaderStage::FRAGMENT: return "ps_6_0";
        case ShaderStage::GEOMETRY: return "gs_6_0";
        case ShaderStage::COMPUTE: return "cs_6_0";
        default:
          ASSERT(false, "Unsupported HLSL shader-profile");
          return "";
        }
      }
      case ShaderModel::SM_6_1:
      {
        switch (aShaderStage)
        {
        case ShaderStage::VERTEX: return "vs_6_1";
        case ShaderStage::FRAGMENT: return "ps_6_1";
        case ShaderStage::GEOMETRY: return "gs_6_1";
        case ShaderStage::COMPUTE: return "cs_6_1";
        default:
          ASSERT(false, "Unsupported HLSL shader-profile");
          return "";
        }
      }
      case ShaderModel::SM_6_2:
      {
        switch (aShaderStage)
        {
        case ShaderStage::VERTEX: return "vs_6_2";
        case ShaderStage::FRAGMENT: return "ps_6_2";
        case ShaderStage::GEOMETRY: return "gs_6_2";
        case ShaderStage::COMPUTE: return "cs_6_2";
        default:
          ASSERT(false, "Unsupported HLSL shader-profile");
          return "";
        }
      }
      case ShaderModel::SM_6_3:
      {
        switch (aShaderStage)
        {
        case ShaderStage::VERTEX: return "vs_6_3";
        case ShaderStage::FRAGMENT: return "ps_6_3";
        case ShaderStage::GEOMETRY: return "gs_6_3";
        case ShaderStage::COMPUTE: return "cs_6_3";
        default:
          ASSERT(false, "Unsupported HLSL shader-profile");
          return "";
        }
      }
      case ShaderModel::SM_6_4:
      {
        switch (aShaderStage)
        {
        case ShaderStage::VERTEX: return "vs_6_4";
        case ShaderStage::FRAGMENT: return "ps_6_4";
        case ShaderStage::GEOMETRY: return "gs_6_4";
        case ShaderStage::COMPUTE: return "cs_6_4";
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
  VertexAttributeSemantic ShaderCompiler::GetVertexAttributeSemantic(const char* aSemanticName)
  {
    if (strncmp(aSemanticName, "POSITION", strlen("POSITION")) == 0)
      return VertexAttributeSemantic::POSITION;
    if (strncmp(aSemanticName, "NORMAL", strlen("NORMAL")) == 0)
      return VertexAttributeSemantic::NORMAL;
    if (strncmp(aSemanticName, "TANGENT", strlen("TANGENT")) == 0)
      return VertexAttributeSemantic::TANGENT;
    if (strncmp(aSemanticName, "BINORMAL", strlen("BINORMAL")) == 0)
      return VertexAttributeSemantic::BINORMAL;
    if (strncmp(aSemanticName, "TEXCOORD", strlen("TEXCOORD")) == 0)
      return VertexAttributeSemantic::TEXCOORD;
    if (strncmp(aSemanticName, "COLOR", strlen("COLOR")) == 0)
      return VertexAttributeSemantic::COLOR;
    
    ASSERT(false, "Unrecognized vertex semantic %s", aSemanticName);
    return VertexAttributeSemantic::NONE;
  }
//---------------------------------------------------------------------------//
  eastl::string ShaderCompiler::GetShaderPathRelative(const char* aPath) const
  {
    const StaticFilePath path("%s/%s", GetShaderRootFolderRelative(), aPath);
    return eastl::string(path);
  }
//---------------------------------------------------------------------------//
  bool ShaderCompiler::Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const
  {
    LOG_INFO("Compiling shader %s...", aDesc.myPath.c_str());

    StaticFilePath hlslSrcPathRel("%s/%s", GetShaderRootFolderRelative(), aDesc.myPath.c_str());
    eastl::string hlslSrcPathAbs = Path::GetAbsoluteResourcePath(hlslSrcPathRel.GetBuffer());

    ASSERT(!hlslSrcPathAbs.empty());
    if (hlslSrcPathAbs.empty())
      return false;
    
    const bool success = Compile_Internal(hlslSrcPathAbs.c_str(), aDesc, aCompilerOutput);
    if (success)
    {
      aCompilerOutput->myDesc = aDesc;
      aCompilerOutput->myProperties.myShaderStage = static_cast<ShaderStage>(aDesc.myShaderStage);
    }
    return success;
  }
 //---------------------------------------------------------------------------//
}
