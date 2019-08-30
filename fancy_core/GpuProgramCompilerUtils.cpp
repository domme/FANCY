#include "fancy_core_precompile.h"
#include "GpuProgramCompilerUtils.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  const char* GpuProgramCompilerUtils::ShaderStageToDefineString(ShaderStage aShaderStage)
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
}