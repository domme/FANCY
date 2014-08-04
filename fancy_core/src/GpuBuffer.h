#ifndef INCLUDE_GPUBUFFER_H
#define INCLUDE_GPUBUFFER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUBUFFER

namespace FANCY { namespace Core { namespace Rendering {

  class GpuBuffer : public PLATFORM_DEPENDENT_NAME(GpuBuffer)
  {

  };

} } } // end of namespace FANCY::Core::Rendering


#endif  // INCLUDE_GPUBUFFER_H