#pragma once

#include "Any.h"
#include "RenderEnums.h"
#include "FancyCoreDefines.h"
#include "TextureData.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct GpuResourceDataDX12;
  struct GpuResourceDataVk;
//---------------------------------------------------------------------------//
  class GpuResource
  {
  public:
    GpuResource(GpuResource&& anOther) = default;

    explicit GpuResource(GpuResourceType aType)
      : mySubresources(0, 1u, 0u, 1u, 0u, 1u)
      , myType(aType)
    {}

    void operator=(GpuResource&& anOtherResource) noexcept
    {
      myType = anOtherResource.myType;
      myNativeData = anOtherResource.myNativeData;
      myName = anOtherResource.myName;
      mySubresources = anOtherResource.mySubresources;
    }

    static uint CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex);
    static uint CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes);

    uint GetSubresourceIndex(const SubresourceLocation& aSubresourceLocation) const;
    SubresourceLocation GetSubresourceLocation(uint aSubresourceIndex) const;
    const SubresourceRange& GetSubresources() const { return mySubresources; }
    const char* GetName() const { return myName.c_str(); }

    GpuResourceDataDX12* GetDX12Data() const;
    GpuResourceDataVk* GetVkData() const;

    bool IsBuffer() const { return myType == GpuResourceType::BUFFER; }
    bool IsTexture() const { return myType == GpuResourceType::TEXTURE; }
    GpuResourceType GetType() const { return myType; }

    virtual ~GpuResource() = default;
    virtual bool IsValid() const { return false; }
    virtual void SetName(const char* aName) { myName = aName; }

    SubresourceRange mySubresources;
    String myName;
    GpuResourceType myType;
    Any myNativeData;
  };
//---------------------------------------------------------------------------//
}
