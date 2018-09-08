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
      : myCoversAllSubresources(false)
      , myResource(aResource)
    {
      memset(mySubresourceOffsets, 0u, sizeof(mySubresourceOffsets));
      memset(myNumSubresources, 0u, sizeof(myNumSubresources));
    }

    bool myCoversAllSubresources;
    uint mySubresourceOffsets[ourNumSupportedPlanes];
    uint myNumSubresources[ourNumSupportedPlanes];
    SharedPtr<GpuResource> myResource;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
