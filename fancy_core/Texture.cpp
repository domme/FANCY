#include "fancy_core_precompile.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  Texture::Texture()
    : GpuResource(GpuResourceCategory::TEXTURE)
  {
  }
//---------------------------------------------------------------------------//
  Texture::Texture(GpuResource&& aResource, const TextureProperties& someProperties)
    : GpuResource(std::move(aResource))
    , myProperties(someProperties)
  {
  }
//---------------------------------------------------------------------------//
  uint Texture::CalcSubresourceIndex(uint aMipIndex, uint aNumMips, uint anArrayIndex, uint aNumArraySlices, uint aPlaneIndex)
  {
    return aPlaneIndex * aNumMips * aNumArraySlices +
      anArrayIndex * aNumMips +
      aMipIndex;
  }
  //---------------------------------------------------------------------------//
  uint Texture::CalcNumSubresources(uint aNumMips, uint aNumArraySlices, uint aNumPlanes)
  {
    return aNumMips * aNumArraySlices * aNumPlanes;
  }
//---------------------------------------------------------------------------//
  uint Texture::GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const
  {
    const uint index = CalcSubresourceIndex(aSubresourceLocation.myMipLevel, myProperties.myNumMipLevels,
      aSubresourceLocation.myArrayIndex, myProperties.GetArraySize(),
      aSubresourceLocation.myPlaneIndex);

    return index;
  }
  //---------------------------------------------------------------------------//
  TextureSubLocation Texture::GetSubresourceLocation(uint aSubresourceIndex) const
  {
    ASSERT(aSubresourceIndex < myNumSubresources);

    TextureSubLocation location;
    location.myMipLevel = aSubresourceIndex % myProperties.myNumMipLevels;
    location.myArrayIndex = (aSubresourceIndex / myProperties.myNumMipLevels) % myProperties.GetArraySize();
    location.myPlaneIndex = aSubresourceIndex / (myProperties.myNumMipLevels * myProperties.GetArraySize());
    return location;
  }


}
