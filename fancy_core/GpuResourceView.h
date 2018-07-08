#pragma once

#include "GpuResource.h"
#include "Any.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceView
  {
    GpuResourceView(const SharedPtr<GpuResource>& aResource)
      : myResource(aResource) 
    { }

    SharedPtr<GpuResource> myResource;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
