#pragma once

#include "GpuResource.h"
#include "Any.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class GpuResourceView
  {
  public:
    GpuResourceView(const SharedPtr<GpuResource>& aResource)
      : myResource(aResource) 
    { }

    SharedPtr<GpuResource> myResource;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
