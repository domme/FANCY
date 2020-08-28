#pragma once

#include "GpuResource.h"
#include "Ptr.h"

#include "EASTL/any.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceViewType {
    NONE = 0,
    CBV, SRV, UAV, DSV, RTV
  };
//---------------------------------------------------------------------------//
  class GpuResourceView
  {
  public:
    virtual ~GpuResourceView() = default;

    explicit GpuResourceView(SharedPtr<GpuResource> aResource)
      : myResource(aResource)
      , myCoversAllSubresources(true)
      , myType(GpuResourceViewType::NONE)
    { }

    GpuResource* GetResource() const { return myResource.get(); }
    const SubresourceRange& GetSubresourceRange() const { return mySubresourceRange; }

    eastl::any myNativeData;
    SubresourceRange mySubresourceRange;
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;
    GpuResourceViewType myType;
  };
//---------------------------------------------------------------------------//
}
