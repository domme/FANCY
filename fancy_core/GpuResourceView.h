#pragma once

#include "GpuResource.h"
#include "Any.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceView
  {
  public:
    static const uint ourNumSupportedPlanes = 2u;

    explicit GpuResourceView(const SharedPtr<GpuResource>& aResource)
      : myResource(aResource)
      , myCoversAllSubresources(true)
    { }

    DynamicArray<uint16> mySubresources[ourNumSupportedPlanes];
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;

    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
