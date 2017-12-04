#pragma once

#include "RendererPrerequisites.h"
#include "Descriptor.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class GpuResourceCategory
  {
    TEXTURE = 0,
    BUFFER
  };  
//---------------------------------------------------------------------------//
  class DLLEXPORT GpuResource
  {
  public:
    GpuResource(GpuResourceCategory aType)
      : myCategory(aType)   
      , myUsageState(GpuResourceState::RESOURCE_STATE_COMMON)
    {}

    virtual ~GpuResource() = default;
    virtual bool IsValid() const = 0;
    virtual const Descriptor* GetDescriptor(DescriptorType aType, uint anIndex = 0u) const = 0;

    GpuResourceCategory myCategory;
    GpuResourceState myUsageState;
  };
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering