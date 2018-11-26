#pragma once

#include "Any.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceCategory
  {
    TEXTURE = 0,
    BUFFER
  };
//---------------------------------------------------------------------------//
  class GpuResource
  {
  public:
    explicit GpuResource(GpuResourceCategory aType)
      : myCategory(aType)
    {}

    void operator=(GpuResource&& anOtherResource)
    {
      myCategory = anOtherResource.myCategory;
      myNativeData = std::move(anOtherResource.myNativeData);
      myName = std::move(anOtherResource.myName);
    }

    virtual ~GpuResource() = default;
    virtual bool IsValid() const { return false; }
    virtual void SetName(const char* aName) { myName = aName; }

    String myName;
    GpuResourceCategory myCategory;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
