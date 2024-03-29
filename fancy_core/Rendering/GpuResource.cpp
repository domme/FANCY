#include "fancy_core_precompile.h"
#include "GpuResource.h"
#include "DX12/GpuResourceDataDX12.h"
#include "Vulkan/GpuResourceDataVk.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  uint GpuResource::CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex)
  {
    return aPlaneIndex * aNumMips * aNumArraySlices +
      anArrayIndex * aNumMips +
      aMipIndex;
  }
//---------------------------------------------------------------------------//
  uint GpuResource::CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes)
  {
    return aNumMips * aNumArraySlices * aNumPlanes;
  }
//---------------------------------------------------------------------------//
  uint GpuResource::GetSubresourceIndex(const SubresourceLocation& aSubresourceLocation) const
  {
    const uint index = CalcSubresourceIndex(aSubresourceLocation.myMipLevel, mySubresources.myNumMipLevels,
      aSubresourceLocation.myArrayIndex, mySubresources.myNumArrayIndices,
      aSubresourceLocation.myPlaneIndex);

    return index;
  }
//---------------------------------------------------------------------------//
  SubresourceLocation GpuResource::GetSubresourceLocation(uint aSubresourceIndex) const
  {
    SubresourceLocation location;
    location.myMipLevel = aSubresourceIndex % mySubresources.myNumMipLevels;
    location.myArrayIndex = (aSubresourceIndex / mySubresources.myNumMipLevels) % mySubresources.myNumArrayIndices;
    location.myPlaneIndex = aSubresourceIndex / (mySubresources.myNumMipLevels * mySubresources.myNumArrayIndices);

    ASSERT(location.myMipLevel < mySubresources.myNumMipLevels 
      && location.myArrayIndex < mySubresources.myNumArrayIndices 
      && location.myPlaneIndex < mySubresources.myNumPlanes);

    return location;
  }
//---------------------------------------------------------------------------//
#if FANCY_ENABLE_DX12
  GpuResourceDataDX12* GpuResource::GetDX12Data() const
  {
    return !myNativeData.has_value() ? nullptr : const_cast<GpuResourceDataDX12*>(eastl::any_cast<GpuResourceDataDX12>(&myNativeData));
  }
#endif
//---------------------------------------------------------------------------//
#if FANCY_ENABLE_VK
  GpuResourceDataVk* GpuResource::GetVkData() const
  {
    return !myNativeData.has_value() ? nullptr : const_cast<GpuResourceDataVk*>(eastl::any_cast<GpuResourceDataVk>(&myNativeData));
  }
#endif
//---------------------------------------------------------------------------//
}