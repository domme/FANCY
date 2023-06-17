#pragma once

#include "GpuResource.h"
#include "Common/Ptr.h"

#include "EASTL/any.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceViewType {
    NONE = 0,
    CBV, SRV, SRV_RT_AS, UAV, DSV, RTV
  };
//---------------------------------------------------------------------------//
  class GpuResourceView
  {
  public:
    explicit GpuResourceView(SharedPtr<GpuResource> aResource, const char* aName)
      : myResource(aResource)
      , myCoversAllSubresources(true)
      , myType(GpuResourceViewType::NONE)
      , myGlobalDescriptorIndex(UINT_MAX)
      , myName(aName != nullptr ? aName : "")
    { }

    virtual ~GpuResourceView() = default;

    GpuResource* GetResource() const { return myResource.get(); }
    const SubresourceRange& GetSubresourceRange() const { return mySubresourceRange; }
    GpuResourceViewType GetType() const { return myType; }
    uint GetGlobalDescriptorIndex() const { return myGlobalDescriptorIndex; }

    eastl::any myNativeData;
    SubresourceRange mySubresourceRange;
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;
    GpuResourceViewType myType;
    uint myGlobalDescriptorIndex;
    eastl::string myName;
  };
//---------------------------------------------------------------------------//
}
