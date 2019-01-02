#pragma once

#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerUtils
  {
  public:
    static String ShaderStageToDefineString(ShaderStage aShaderStage);
  };
//---------------------------------------------------------------------------//
}
