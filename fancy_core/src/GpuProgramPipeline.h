#ifndef INCLUDE_GPUPROGRAMPIPELINE_H
#define INCLUDE_GPUPROGRAMPIPELINE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMPIPELINE
#include "StaticManagedObject.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuProgramPipeline : public PLATFORM_DEPENDENT_NAME(GpuProgramPipeline), public StaticManagedHeapObject<GpuProgramPipeline>
  {
    SERIALIZABLE(GpuProgramPipeline);

  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_GPUPROGRAMPIPELINE_H