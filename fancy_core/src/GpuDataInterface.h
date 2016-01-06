#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_GPUDATAINTERFACE

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------// 
  class GpuDataInterface : public PLATFORM_DEPENDENT_NAME(GpuDataInterface)
  {
    
  };
//---------------------------------------------------------------------------//
  DECLARE_SMART_PTRS(GpuDataInterface);
//---------------------------------------------------------------------------//
} }
