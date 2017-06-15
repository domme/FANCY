#pragma once

#include "RendererPrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class GpuResourceCategory
  {
    TEXTURE = 0,
    BUFFER
  };
//---------------------------------------------------------------------------//
  class GpuResource
  {
  public:
    GpuResource(GpuResourceCategory aType)
      : myCategory(aType)   
    {}

    virtual ~GpuResource() = default;
    virtual bool IsValid() const = 0;

    GpuResourceCategory myCategory;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering