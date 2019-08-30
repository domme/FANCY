#pragma once

#include "RenderEnums.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerUtils
  {
  public:
    GpuProgramCompilerUtils() = delete;

    static const char* ShaderStageToDefineString(ShaderStage aShaderStage);
  };
//---------------------------------------------------------------------------//
}
