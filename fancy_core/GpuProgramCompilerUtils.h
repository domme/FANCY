#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerUtils
  {
  public:
    static String ShaderStageToDefineString(ShaderStage aShaderStage);
  };
//---------------------------------------------------------------------------//
}
