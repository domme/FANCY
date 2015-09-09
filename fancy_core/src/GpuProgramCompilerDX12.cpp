#include "StdAfx.h"

#if defined (RENDERER_DX12)

#include "GpuProgramCompilerDX12.h"

namespace Fancy {
  namespace Rendering {
    namespace DX12 {

    GpuProgram* GpuProgramCompilerDX12::createOrRetrieve(const String& _shaderPath, const GpuProgramPermutation& _permutation, ShaderStage _eShaderStage)
    {
      return nullptr;
    }

    GpuProgramCompilerDX12::GpuProgramCompilerDX12()
      {
      }


      GpuProgramCompilerDX12::~GpuProgramCompilerDX12()
      {
      }

    }
  }
}

#endif
