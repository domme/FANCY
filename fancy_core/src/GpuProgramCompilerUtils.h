#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"

namespace Fancy {namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerUtils
  {
  public:
    static String ShaderStageToDefineString(ShaderStage aShaderStage);
  };
//---------------------------------------------------------------------------//
} }
