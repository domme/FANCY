#pragma once

#include "Any.h"
#include "RenderEnums.h"
#include "DynamicArray.h"
#include "FancyCoreDefines.h"
#include "GpuResourceStateTracking.h"
#include "TextureData.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResource
  {
  public:
    GpuResource(GpuResource&& anOther) = default;

    explicit GpuResource(GpuResourceCategory aType)
      : mySubresources(0, 1u, 0u, 1u, 0u, 1u)
      , myCategory(aType)
    {}

    void operator=(GpuResource&& anOtherResource) noexcept
    {
      myCategory = anOtherResource.myCategory;
      myNativeData = anOtherResource.myNativeData;
      myName = anOtherResource.myName;
      myStateTracking = anOtherResource.myStateTracking;
      mySubresources = anOtherResource.mySubresources;
    }

    static uint CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex);
    static uint CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes);
    uint GetSubresourceIndex(const SubresourceLocation& aSubresourceLocation) const;
    SubresourceLocation GetSubresourceLocation(uint aSubresourceIndex) const;
    const SubresourceRange& GetSubresources() const { return mySubresources; }

    GpuResourceState GetDefaultState() const { return myStateTracking.myDefaultState; }

    virtual ~GpuResource() = default;
    virtual bool IsValid() const { return false; }
    virtual void SetName(const char* aName) { myName = aName; }

    SubresourceRange mySubresources;
    String myName;
    GpuResourceCategory myCategory;
    mutable GpuResourceStateTracking myStateTracking;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
