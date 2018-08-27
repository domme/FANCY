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
    }

    virtual ~GpuResource() = default;
    
    bool IsValid() const { return myStorage != nullptr; }

    GpuResourceCategory myCategory;
    UniquePtr<GpuResourceStorage> myStorage;
  };
//---------------------------------------------------------------------------//
}