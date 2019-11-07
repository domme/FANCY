#include "fancy_core_precompile.h"
#include "TextureData.h"
#include "TextureProperties.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  TextureRegion TextureRegion::ourMaxRegion;
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  bool SubresourceRange::IsEmpty() const
  {
    return myNumMipLevels == 0u && myNumArrayIndices == 0u && myNumPlanes == 0u;
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
        if (myCurrentLocation.myPlaneIndex >= myEndLocation.myPlaneIndex)
        {
          myCurrentLocation.myPlaneIndex = myRange.myFirstPlane;
        }
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
    return myCurrentLocation.myMipLevel >= myRange.myNumMipLevels
      || myCurrentLocation.myArrayIndex >= myRange.myNumArrayIndices
      || myCurrentLocation.myPlaneIndex >= myRange.myNumPlanes;
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  TextureSubData::TextureSubData(const TextureProperties& someProperties)
    : myData(nullptr)
  {
    const DataFormatInfo info(someProperties.eFormat);
    myPixelSizeBytes = info.mySizeBytes;

    const uint width = someProperties.myWidth;
    const uint height = glm::max(1u, someProperties.myHeight);
    const uint depth = glm::max(1u, someProperties.myDepthOrArraySize);

    myTotalSizeBytes = myPixelSizeBytes * width * height * depth;
    mySliceSizeBytes = myPixelSizeBytes * width * height;
    myRowSizeBytes = myPixelSizeBytes * width;
  }
//---------------------------------------------------------------------------//
}
