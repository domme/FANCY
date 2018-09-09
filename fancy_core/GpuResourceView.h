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
      : mySubresourceOffsets{ 0u }
      , myNumSubresources{ UINT_MAX }
      , myResource(aResource)
      , myCoversAllSubresources(true)
    { }

    uint mySubresourceOffsets[ourNumSupportedPlanes];
    uint myNumSubresources[ourNumSupportedPlanes];
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
