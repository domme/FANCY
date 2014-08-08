#ifndef INCLUDE_GPUBUFFER_H
#define INCLUDE_GPUBUFFER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUBUFFER

namespace Fancy { namespace Core { namespace Rendering {

  class GpuBuffer : public PLATFORM_DEPENDENT_NAME(GpuBuffer)
  {

  };

} } } // end of namespace Fancy::Core::Rendering


#endif  // INCLUDE_GPUBUFFER_H