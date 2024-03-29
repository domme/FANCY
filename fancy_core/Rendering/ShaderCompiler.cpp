#include "fancy_core_precompile.h"
#include "ShaderCompiler.h"
#include "IO/PathService.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  const char* ShaderCompiler::ShaderStageToDefineString(ShaderStage aShaderStage)
  {
    switch (aShaderStage)
    {
    case SHADERSTAGE_VERTEX:
      return "PROGRAM_TYPE_VERTEX";
    case SHADERSTAGE_FRAGMENT:
      return "PROGRAM_TYPE_FRAGMENT";
    case SHADERSTAGE_GEOMETRY:
      return "PROGRAM_TYPE_GEOMETRY";
    case SHADERSTAGE_TESS_HULL:
      return "PROGRAM_TYPE_TESS_HULL";
    case SHADERSTAGE_TESS_DOMAIN:
      return "PROGRAM_TYPE_TESS_DOMAIN";
    case SHADERSTAGE_COMPUTE:
      return "PROGRAM_TYPE_COMPUTE";
    case SHADERSTAGE_RAYGEN:
      return "PROGRAM_TYPE_RAYGEN";
    case SHADERSTAGE_MISS:
      return "PROGRAM_TYPE_MISS";
    case SHADERSTAGE_INTERSECTION:
      return "PROGRAM_TYPE_INTERSECTION";
    case SHADERSTAGE_ANYHIT: 
      return "PROGRAM_TYPE_ANYHIT";
    case SHADERSTAGE_CLOSEST_HIT: 
      return "PROGRAM_TYPE_CLOSEST_HIT";
    default:
      ASSERT(false); return "";
    }
  }
//---------------------------------------------------------------------------//
  const char* locGetShaderStagePrefix(ShaderStage aShaderStage)
  {
    switch (aShaderStage)
    {
    case SHADERSTAGE_VERTEX: 
      return "vs_";
    case SHADERSTAGE_FRAGMENT: 
      return "ps_";
    case SHADERSTAGE_GEOMETRY: 
      return "gs_";
    case SHADERSTAGE_TESS_HULL: 
      return "hs_";
    case SHADERSTAGE_TESS_DOMAIN: 
      return "ds_";
    case SHADERSTAGE_COMPUTE: 
      return "cs_";
    case SHADERSTAGE_RAYGEN: 
    case SHADERSTAGE_MISS: 
    case SHADERSTAGE_INTERSECTION: 
    case SHADERSTAGE_ANYHIT: 
    case SHADERSTAGE_CLOSEST_HIT:
      return "lib_";
    default: ASSERT(false); return "";
    }
  }
//---------------------------------------------------------------------------//
  const char* locGetShaderModelSuffix(ShaderModel aShaderModel)
  {
    switch(aShaderModel)
    {
    case SM_6_0: return "6_0";
    case SM_6_1: return "6_1";
    case SM_6_2: return "6_2";
    case SM_6_3: return "6_3";
    case SM_6_4: return "6_4";
    case SM_6_5: return "6_5";
    default: ASSERT(false); return "";
    }
  }
//---------------------------------------------------------------------------//
  FixedShortString ShaderCompiler::GetHLSLprofileString(ShaderStage aShaderStage, ShaderModel aShaderModel)
  {
    FixedShortString str;
    str.sprintf("%s%s", locGetShaderStagePrefix(aShaderStage), locGetShaderModelSuffix(aShaderModel));
    return str;
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
  bool ShaderCompiler::Compile(const ShaderDesc& aDesc, ShaderCompilerResult* aCompilerOutput) const
  {
    eastl::string hlslSrcPathAbs = Path::GetAbsolutePath(aDesc.myPath.c_str());
    LOG_INFO("Compiling shader %s...", hlslSrcPathAbs.c_str());

    ASSERT(!hlslSrcPathAbs.empty());
    if (hlslSrcPathAbs.empty())
      return false;

    if (!Path::FileExists(hlslSrcPathAbs.c_str()))
      LOG_ERROR("Shader file not found at %s", hlslSrcPathAbs.c_str());
    
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
