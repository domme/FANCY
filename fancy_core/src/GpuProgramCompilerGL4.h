#ifndef INCLUDE_GPUPROGRAMCOMPILERGL4_H
#define INCLUDE_GPUPROGRAMCOMPILERGL4_H

#if defined (RENDERER_OPENGL4)

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramFeatures.h"

namespace Fancy{ namespace Rendering{
  struct GpuProgramDesc;
} }

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  struct GpuProgramCompilerOutputGL4;
//---------------------------------------------------------------------------//
  class GpuProgramCompilerGL4
  {
    public:
      static bool Compile(const GpuProgramDesc& aDesc, GpuProgramCompilerOutputGL4& aCompilerOutput);
      static GpuProgram* createOrRetrieve(const GpuProgramDesc& aDesc);
      static bool compileFromSource(const String& someShaderSource, const ShaderStage& eShaderStage, GLuint& aProgramHandleGL);

      // TODO: Find a nicer place for platform-dependent infos
      static String GetPlatformShaderFileExtension() { return ".shader"; }
      static String GetPlatformShaderFileDirectory() { return "shader/GL4/"; }
      
    private:
      GpuProgramCompilerGL4();
      ~GpuProgramCompilerGL4();
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4

#endif

#endif  // INCLUDE_GPUPROGRAMCOMPILERGL4_H