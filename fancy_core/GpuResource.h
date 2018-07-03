#pragma once

#include "RendererPrerequisites.h"
#include "GpuResourceStorage.h"

namespace Fancy {
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
      , myUsageState(GpuResourceState::RESOURCE_STATE_COMMON)
    {}

    virtual ~GpuResource() = default;
    
    bool IsValid() const { return myStorage != nullptr; }

    GpuResourceCategory myCategory;
    GpuResourceState myUsageState;  // TODO: Support resource-states per subresource
    UniquePtr<GpuResourceStorage> myStorage;
  };
//---------------------------------------------------------------------------//
}