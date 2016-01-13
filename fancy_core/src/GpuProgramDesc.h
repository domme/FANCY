#pragma once
#include "FancyCorePrerequisites.h"
#include "GpuProgram.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  struct GpuProgramDesc
  {
    String myShaderPath;
    uint myShaderStage;
    GpuProgramPermutation myPermutation;
  };
//---------------------------------------------------------------------------//  
  struct GpuProgramPipelineDesc
  {
    GpuProgramDesc myGpuPrograms[(uint32)ShaderStage::NUM];
  };
//---------------------------------------------------------------------------//
} }

