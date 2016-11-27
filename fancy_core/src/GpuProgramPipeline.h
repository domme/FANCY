#ifndef INCLUDE_GPUPROGRAMPIPELINE_H
#define INCLUDE_GPUPROGRAMPIPELINE_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "GpuProgramPipelineDesc.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUPROGRAMPIPELINE

#include "Serializable.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuProgramPipeline : public PLATFORM_DEPENDENT_NAME(GpuProgramPipeline)
  {
  public:
    SERIALIZABLE_RESOURCE(GpuProgramPipeline);

  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering

#endif  // INCLUDE_GPUPROGRAMPIPELINE_H