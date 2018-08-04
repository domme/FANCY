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
    explicit GpuResource(GpuResourceCategory aType)
      : myCategory(aType)   
      , myUsageState(GpuResourceState::RESOURCE_STATE_COMMON)
    {}

    void operator=(GpuResource&& anOtherResource)
    {
      myCategory = anOtherResource.myCategory;
      myUsageState = anOtherResource.myUsageState;
      myStorage = std::move(anOtherResource.myStorage);
    }

    virtual ~GpuResource() = default;
    
    bool IsValid() const { return myStorage != nullptr; }

    GpuResourceCategory myCategory;
    GpuResourceState myUsageState;  // TODO: Support resource-states per subresource
    UniquePtr<GpuResourceStorage> myStorage;
  };
//---------------------------------------------------------------------------//
}