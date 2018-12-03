#pragma once

#include "Any.h"
#include "CommandListType.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class GpuResourceCategory
  {
    TEXTURE = 0,
    BUFFER
  };
//---------------------------------------------------------------------------//
  struct GpuHazardData
  {
    GpuHazardData()
      : myCanChangeStates(true)
      , myAllSubresourcesInSameState(true)
      , mySubresourceContexts{ CommandListType::Graphics }
    {}

    bool myCanChangeStates;
    bool myAllSubresourcesInSameState;
    DynamicArray<CommandListType> mySubresourceContexts;
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
    UniquePtr<GpuHazardData> myHazardData;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
