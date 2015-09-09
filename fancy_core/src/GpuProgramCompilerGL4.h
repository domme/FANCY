#ifndef INCLUDE_GPUPROGRAMCOMPILERGL4_H
#define INCLUDE_GPUPROGRAMCOMPILERGL4_H

#if defined (RENDERER_OPENGL4)

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramFeatures.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerGL4
  {
    public:
      static GpuProgram* createOrRetrieve(const String& _shaderPath, const GpuProgramPermutation& _permutation, ShaderStage _eShaderStage);
      
    private:
      GpuProgramCompilerGL4();
      ~GpuProgramCompilerGL4();

      static bool compileFromSource(const String& someShaderSource, const ShaderStage& eShaderStage, GLuint& aProgramHandleGL);
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4

#endif

#endif  // INCLUDE_GPUPROGRAMPIPELINEGL4_H