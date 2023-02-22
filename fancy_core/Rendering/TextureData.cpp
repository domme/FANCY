#include "fancy_core_precompile.h"
#include "TextureData.h"
#include "TextureProperties.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  bool SubresourceRange::IsEmpty() const
  {
    return myNumMipLevels == 0u && myNumArrayIndices == 0u && myNumPlanes == 0u;
  }
//---------------------------------------------------------------------------//
  uint SubresourceRange::GetNumSubresources() const
  {
    return myNumMipLevels * myNumArrayIndices * myNumPlanes;
  }
//---------------------------------------------------------------------------//
  uint SubresourceRange::GetNumSubresourcesPerPlane() const
  {
    return myNumMipLevels * myNumArrayIndices;
  }
//---------------------------------------------------------------------------//
  bool SubresourceRange::operator==(const SubresourceRange& anOther) const
  {
    return myFirstMipLevel == anOther.myFirstMipLevel
      && myFirstArrayIndex == anOther.myFirstArrayIndex
      && myFirstPlane == anOther.myFirstPlane
      && myNumMipLevels == anOther.myNumMipLevels
      && myNumArrayIndices == anOther.myNumArrayIndices
      && myNumPlanes == anOther.myNumPlanes;
  }
//---------------------------------------------------------------------------//
  SubresourceIterator SubresourceRange::Begin() const
  {
    return SubresourceIterator(*this);
  }
  //---------------------------------------------------------------------------//
  SubresourceIterator SubresourceRange::End() const
  {
    SubresourceLocation endLocation;
    endLocation.myMipLevel = myNumMipLevels + 1u;
    endLocation.myArrayIndex = myNumArrayIndices + 1u;
    endLocation.myPlaneIndex = myNumPlanes + 1u;
    return SubresourceIterator(*this, endLocation);
  }
//---------------------------------------------------------------------------//
  SubresourceIterator::SubresourceIterator(const SubresourceRange& aRange)
    : myRange(aRange)
  {
    myCurrentLocation.myMipLevel = myRange.myFirstMipLevel;
    myCurrentLocation.myArrayIndex = myRange.myFirstArrayIndex;
    myCurrentLocation.myPlaneIndex = myRange.myFirstPlane;

    myEndLocation.myMipLevel = myRange.myFirstMipLevel + myRange.myNumMipLevels;
    myEndLocation.myArrayIndex = myRange.myFirstArrayIndex + myRange.myNumArrayIndices;
    myEndLocation.myPlaneIndex = myRange.myFirstPlane + myRange.myNumPlanes;
  }
//---------------------------------------------------------------------------//
  SubresourceIterator::SubresourceIterator(const SubresourceRange& aRange, SubresourceLocation aLocation)
    : myRange(aRange)
    , myCurrentLocation(aLocation)
  {
    myEndLocation.myMipLevel = myRange.myFirstMipLevel + myRange.myNumMipLevels;
    myEndLocation.myArrayIndex = myRange.myFirstArrayIndex + myRange.myNumArrayIndices;
    myEndLocation.myPlaneIndex = myRange.myFirstPlane + myRange.myNumPlanes;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  const SubresourceLocation& SubresourceIterator::operator++()
  {
    ++myCurrentLocation.myMipLevel;

    if (myCurrentLocation.myMipLevel >= myEndLocation.myMipLevel)
    {
      myCurrentLocation.myMipLevel = myRange.myFirstMipLevel;

      ++myCurrentLocation.myArrayIndex;
      if (myCurrentLocation.myArrayIndex >= myEndLocation.myArrayIndex)
      {
        myCurrentLocation.myArrayIndex = myRange.myFirstArrayIndex;

        ++myCurrentLocation.myPlaneIndex;
      }
    }

    if (IsEnd())
      return myEndLocation;

    return myCurrentLocation;
  }
//---------------------------------------------------------------------------//
  bool SubresourceIterator::operator==(const SubresourceIterator& anOther) const
  {
    if (IsEnd() && anOther.IsEnd())
      return true;

    return myCurrentLocation.myMipLevel == anOther.myCurrentLocation.myMipLevel
      && myCurrentLocation.myArrayIndex == anOther.myCurrentLocation.myArrayIndex
      && myCurrentLocation.myPlaneIndex == anOther.myCurrentLocation.myPlaneIndex;
  }
//---------------------------------------------------------------------------//
  bool SubresourceIterator::operator!=(const SubresourceIterator& anOther) const
  {
    return !(*this == anOther);
  }
//---------------------------------------------------------------------------//
  bool SubresourceIterator::IsEnd() const
  {
    return myCurrentLocation.myMipLevel >= myRange.myFirstMipLevel + myRange.myNumMipLevels
      || myCurrentLocation.myArrayIndex >= myRange.myFirstArrayIndex + myRange.myNumArrayIndices
      || myCurrentLocation.myPlaneIndex >= myRange.myFirstPlane + myRange.myNumPlanes;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  TextureSubData::TextureSubData(const TextureProperties& someProperties)
    : myData(nullptr)
  {
    const uint width = someProperties.myWidth;
    const uint height = glm::max(1u, someProperties.myHeight);
    const uint depth = glm::max(1u, someProperties.myDepthOrArraySize);

    uint64 pitchSizeBytes;
    uint heightBlocksOrPixels;
    TextureData::ComputeRowPitchSizeAndBlockHeight(someProperties.myFormat, width, height, pitchSizeBytes, heightBlocksOrPixels);

    myRowSizeBytes = pitchSizeBytes;
    mySliceSizeBytes = heightBlocksOrPixels * myRowSizeBytes;
    myTotalSizeBytes = depth * mySliceSizeBytes;
  }
  //---------------------------------------------------------------------------//
  void TextureData::ComputeRowPitchSizeAndBlockHeight(DataFormat aFormat, uint aWidth, uint aHeight, uint64& rowPitchSize, uint& aHeightBlocksOrPixels, uint aPlane /*= 0*/)
  {
    const DataFormatInfo formatInfo(aFormat);
    if (formatInfo.myIsCompressed)
      rowPitchSize = static_cast<uint64>(glm::max(1u, MathUtil::DivideRoundUp(aWidth, 4u)) * formatInfo.myCompressedBlockSizeBytes);
    else
      rowPitchSize = static_cast<uint64>(MathUtil::DivideRoundUp(aWidth * formatInfo.myCopyableBitsPerPixelPerPlane[aPlane], 8u));

    aHeightBlocksOrPixels = formatInfo.myIsCompressed ? MathUtil::DivideRoundUp(aHeight, 4u) : aHeight;
  }
  //---------------------------------------------------------------------------//
}
