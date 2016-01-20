#ifndef INCLUDE_GPUBUFFER_H
#define INCLUDE_GPUBUFFER_H

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "StaticManagedObject.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUBUFFER

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuBuffer : public PLATFORM_DEPENDENT_NAME(GpuBuffer), public StaticManagedHeapObject<GpuBuffer>
  {

  };
//---------------------------------------------------------------------------//
} } // end of namespace Fancy::Rendering


#endif  // INCLUDE_GPUBUFFER_H