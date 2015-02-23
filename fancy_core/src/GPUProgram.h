#ifndef INCLUDE_GPUPROGRAM_H
#define INCLUDE_GPUPROGRAM_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAM

namespace Fancy { namespace Rendering {

  class GpuProgram : public PLATFORM_DEPENDENT_NAME(GpuProgram), public StaticManagedHeapObject<GpuProgram>
  {

  };

} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_GPUPROGRAM_H