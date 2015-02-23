#ifndef INCLUDE_GPUPROGRAMCOMPILERGL4_H
#define INCLUDE_GPUPROGRAMCOMPILERGL4_H

#include <list>

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "FixedArray.h"
#include "ObjectName.h"
#include "GpuProgramResource.h"
#include "VertexInputLayout.h"
#include "GPUProgramGL4.h"
#include "AdapterGL4.h"

namespace Fancy { namespace Rendering { namespace GL4 {
//---------------------------------------------------------------------------//
  class GpuProgramCompilerGL4
  {
    public:
      static GpuProgram* createOrRetrieve(const String& _shaderPath, ShaderStage _eShaderStage);

    private:
      GpuProgramCompilerGL4();
      ~GpuProgramCompilerGL4();
  };
//---------------------------------------------------------------------------//
} } } // end of namespace Fancy::Rendering:GL4

#endif  // INCLUDE_GPUPROGRAMPIPELINEGL4_H