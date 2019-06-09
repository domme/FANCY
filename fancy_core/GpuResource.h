#pragma once

#include "Any.h"
#include "RenderEnums.h"
#include "DynamicArray.h"
#include "FC_String.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceCategory
  {
    TEXTURE = 0,
    BUFFER
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardDataDX12
  {
    // Uints are D3D12_RESOURCE_STATES
    uint myReadState = 0u;
    DynamicArray<uint> mySubresourceStates;
  };
//---------------------------------------------------------------------------//
  struct GpuResourceHazardData
  {
    bool myCanChangeStates = true;
    bool myAllSubresourcesInSameState = true;
    DynamicArray<CommandListType> mySubresourceContexts;
    GpuResourceHazardDataDX12 myDx12Data;  // Will become a union once other platforms are in
  };
//---------------------------------------------------------------------------//
  class GpuResource
  {
  public:
    explicit GpuResource(GpuResourceCategory aType)
      : myCategory(aType)
    {}

    void operator=(GpuResource&& anOtherResource) noexcept
    {
      myCategory = anOtherResource.myCategory;
      myNativeData = std::move(anOtherResource.myNativeData);
      myName = std::move(anOtherResource.myName);
      myHazardData = std::move(anOtherResource.myHazardData);
    }

    virtual ~GpuResource() = default;
    virtual bool IsValid() const { return false; }
    virtual void SetName(const char* aName) { myName = aName; }
    virtual uint GetNumSubresources() const { return 1u; }

    String myName;
    GpuResourceCategory myCategory;
    mutable GpuResourceHazardData myHazardData;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
