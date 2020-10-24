#pragma once

#include "GpuResource.h"
#include "Ptr.h"
#include "RenderCore.h"
#include "Slot.h"

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
    explicit GpuResourceView(SharedPtr<GpuResource> aResource)
      : myResource(aResource)
      , myCoversAllSubresources(true)
      , myType(GpuResourceViewType::NONE)
    { }

    virtual ~GpuResourceView()
    {
      myOnDestroyed(this);
    }

    GpuResource* GetResource() const { return myResource.get(); }
    const SubresourceRange& GetSubresourceRange() const { return mySubresourceRange; }
    GpuResourceViewType GetType() const { return myType; }

    eastl::any myNativeData;
    SubresourceRange mySubresourceRange;
    SharedPtr<GpuResource> myResource;
    bool myCoversAllSubresources;
    GpuResourceViewType myType;
    mutable Slot<void(GpuResourceView*)> myOnDestroyed;
  };
//---------------------------------------------------------------------------//
}
