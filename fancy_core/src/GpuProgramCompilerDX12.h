#pragma once

#if defined (RENDERER_DX12)

namespace Fancy{ namespace Rendering{
class GpuProgramPermutation;
class GpuProgram;
}}

namespace Fancy {
  namespace Rendering {
    namespace DX12 {

      class GpuProgramCompilerDX12
      {
        public:
          static GpuProgram* createOrRetrieve(const String& _shaderPath, const GpuProgramPermutation& _permutation, ShaderStage _eShaderStage);

        private:
          GpuProgramCompilerDX12();
          ~GpuProgramCompilerDX12();
      };

    }
  }
}

#endif 

