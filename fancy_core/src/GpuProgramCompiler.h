#ifndef INCLUDE_GPUPROGRAMCOMPILER_H
#define INCLUDE_GPUPROGRAMCOMPILER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMCOMPILER

namespace Fancy { namespace Core { namespace Rendering {
  
  class GpuProgramCompiler : public PLATFORM_DEPENDENT_NAME(GpuProgramCompiler)
  {

  };

} } } // end of namespace Fancy::Core::Rendering

#endif  // INCLUDE_GPUPROGRAMCOMPILER_H