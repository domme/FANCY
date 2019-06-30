#pragma once

#include "Any.h"
#include "RenderEnums.h"
#include "DynamicArray.h"
#include "FancyCoreDefines.h"

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
      : myNumSubresources(1u)
      , myNumSubresourcesPerPlane(1u)
      , myNumPlanes(1u)
      , myCategory(aType)
    {}

    void operator=(GpuResource&& anOtherResource) noexcept
    {
      myCategory = std::move(anOtherResource.myCategory);
      myNativeData = std::move(anOtherResource.myNativeData);
      myName = std::move(anOtherResource.myName);
      myStateTracking = std::move(anOtherResource.myStateTracking);
      myNumSubresources = std::move(anOtherResource.myNumSubresources);
      myNumSubresourcesPerPlane = std::move(anOtherResource.myNumSubresourcesPerPlane);
      myNumPlanes = std::move(anOtherResource.myNumPlanes);
    }

    virtual ~GpuResource() = default;
    virtual bool IsValid() const { return false; }
    virtual void SetName(const char* aName) { myName = aName; }

    uint myNumSubresources;
    uint myNumSubresourcesPerPlane;
    uint myNumPlanes;

    String myName;
    GpuResourceCategory myCategory;
    mutable GpuResourceStateTracking myStateTracking;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
