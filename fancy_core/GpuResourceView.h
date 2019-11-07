#pragma once

#include "GpuResource.h"
#include "Any.h"
#include "DynamicArray.h"
#include "Ptr.h"

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

    Any myNativeData;
    SubresourceRange mySubresourceRange;
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;
    GpuResourceViewType myType;
  };
//---------------------------------------------------------------------------//
}
