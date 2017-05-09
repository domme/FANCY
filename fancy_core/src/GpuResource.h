#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class GpuResource
  {
  public:
    virtual bool IsValid() const = 0;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering