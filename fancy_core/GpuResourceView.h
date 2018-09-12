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

    // TODO: This design is bad since it doesn't reflect that the resource view might only access a non-continous region of subresources
    uint mySubresourceOffsets[ourNumSupportedPlanes];
    uint myNumSubresources[ourNumSupportedPlanes];
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
