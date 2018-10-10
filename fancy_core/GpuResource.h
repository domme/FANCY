#pragma once

#include "RendererPrerequisites.h"
#include "GpuResourceStorage.h"

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
      myStorage = std::move(anOtherResource.myStorage);
      myName = std::move(anOtherResource.myName);
    }

    virtual ~GpuResource() = default;
    
    bool IsValid() const { return myStorage != nullptr; }

    String myName;
    GpuResourceCategory myCategory;
    UniquePtr<GpuResourceStorage> myStorage;  // TODO: Use Any instead
  };
//---------------------------------------------------------------------------//
}