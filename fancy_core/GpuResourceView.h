#pragma once

#include "RendererPrerequisites.h"
#include "GpuResource.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceView
  {
    GpuResourceView(SharedPtr<GpuResource> aResource, UniquePtr<GpuResourceViewData> aData)
      : myResource(aResource)
      , myNativeData(std::move(aData))
    { }

    GpuResource* GetResource() const { return myResource.get(); }
    GpuResourceViewData* GetNativeData() const { return myNativeData.get(); }

  protected:
    SharedPtr<GpuResource> myResource;
    UniquePtr<GpuResourceViewData> myNativeData;
  };
//---------------------------------------------------------------------------//
}
