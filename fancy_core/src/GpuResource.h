#pragma once

#include "RendererPrerequisites.h"
#include "Descriptor.h"
#include "GpuResourceStorage.h"
#include "ScopedPtr.h"

namespace Fancy { namespace Rendering {
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
    GpuResource(GpuResourceCategory aType)
      : myCategory(aType)   
      , myUsageState(GpuResourceState::RESOURCE_STATE_COMMON)
    {}

    virtual ~GpuResource() = default;
    virtual const Descriptor* GetDescriptor(DescriptorType aType, uint anIndex = 0u) const = 0;

    bool IsValid() const { return myStorage != nullptr; }

    GpuResourceCategory myCategory;
    GpuResourceState myUsageState;
    ScopedPtr<GpuResourceStorage> myStorage;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering