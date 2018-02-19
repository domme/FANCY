#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

#include "GpuProgramCompilerUtils.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  String GpuProgramCompilerUtils::ShaderStageToDefineString(ShaderStage aShaderStage)
  {
    switch (aShaderStage)
    {
    case Fancy::Rendering::ShaderStage::VERTEX: 
      return "PROGRAM_TYPE_VERTEX";
    case Fancy::Rendering::ShaderStage::FRAGMENT:
      return "PROGRAM_TYPE_FRAGMENT";
    case Fancy::Rendering::ShaderStage::GEOMETRY:
      return "PROGRAM_TYPE_GEOMETRY";
    case Fancy::Rendering::ShaderStage::TESS_HULL:
      return "PROGRAM_TYPE_TESS_HULL";
    case Fancy::Rendering::ShaderStage::TESS_DOMAIN:
      return "PROGRAM_TYPE_TESS_DOMAIN";
    case Fancy::Rendering::ShaderStage::COMPUTE:
      return "PROGRAM_TYPE_COMPUTE";
    default:
      return "";
    }
  }
//---------------------------------------------------------------------------//
} }