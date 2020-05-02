#include "fancy_core_precompile.h"
#include "GpuResource.h"
#include "GpuResourceDataDX12.h"
#include "GpuResourceDataVk.h"

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
  GpuResourceDataDX12* GpuResource::GetDX12Data() const
  {
    return myNativeData.To<GpuResourceDataDX12*>();
  }
//---------------------------------------------------------------------------//
  GpuResourceDataVk* GpuResource::GetVkData() const
  {
    return myNativeData.To<GpuResourceDataVk*>();
  }
//---------------------------------------------------------------------------//
}