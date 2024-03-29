#pragma once

#include "Common/FancyCoreDefines.h"
#include "Common/MathIncludes.h"
#include "DataFormat.h"

#include "EASTL/vector.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct TextureProperties;
//---------------------------------------------------------------------------//
  struct SubresourceLocation
  {
    SubresourceLocation(uint aMipLevel = 0u, uint anArrayIndex = 0u, uint aPlaneIndex = 0u)
      : myMipLevel(aMipLevel)
      , myArrayIndex(anArrayIndex)
      , myPlaneIndex(aPlaneIndex) 
    {}

    bool operator==(const SubresourceLocation& anOther) const 
    {
      return myMipLevel == anOther.myMipLevel &&
        myArrayIndex == anOther.myArrayIndex &&
        myPlaneIndex == anOther.myPlaneIndex;
    }

    bool operator !=(const SubresourceLocation& anOther) const
    {
      return !(*this == anOther);
    }

    uint myMipLevel;
    uint myArrayIndex;
    uint myPlaneIndex;
  };
//---------------------------------------------------------------------------//
  struct SubresourceIterator;

  struct SubresourceRange
  {
    SubresourceRange(
      uint aFirstMipLevel = 0u,
      uint aNumMipLevels = 1u,
      uint aFirstArrayIndex = 0u,
      uint aNumArrayIndices = 1u,
      uint aFirstPlaneIndex = 0u,
      uint aNumPlanes = 1u)
      : myFirstMipLevel(aFirstMipLevel)
      , myNumMipLevels(aNumMipLevels)
      , myFirstArrayIndex(aFirstArrayIndex)
      , myNumArrayIndices(aNumArrayIndices)
      , myFirstPlane(aFirstPlaneIndex)
      , myNumPlanes(aNumPlanes)
    { }

    SubresourceRange(const SubresourceLocation& aLocation)
      : myFirstMipLevel(aLocation.myMipLevel)
      , myNumMipLevels(1u)
      , myFirstArrayIndex(aLocation.myArrayIndex)
      , myNumArrayIndices(1u)
      , myFirstPlane(aLocation.myPlaneIndex)
      , myNumPlanes(1u)
    { }

    SubresourceRange(const SubresourceLocation& aFirstLocation, const SubresourceLocation& aLastLocation)
      : myFirstMipLevel(aFirstLocation.myMipLevel)
      , myNumMipLevels(aLastLocation.myMipLevel - aFirstLocation.myMipLevel + 1)
      , myFirstArrayIndex(aFirstLocation.myArrayIndex)
      , myNumArrayIndices(aLastLocation.myArrayIndex - aFirstLocation.myArrayIndex + 1)
      , myFirstPlane(aFirstLocation.myPlaneIndex)
      , myNumPlanes(aLastLocation.myPlaneIndex - aFirstLocation.myPlaneIndex + 1)
    { }

    bool operator==(const SubresourceRange& anOther) const;

    SubresourceIterator Begin() const;
    SubresourceIterator End() const;
    SubresourceLocation First() const { return SubresourceLocation(myFirstMipLevel, myFirstArrayIndex, myFirstPlane); }
    SubresourceLocation Last() const { return SubresourceLocation(myFirstMipLevel + myNumMipLevels - 1, myFirstArrayIndex + myNumArrayIndices - 1, myFirstPlane + myNumPlanes - 1); }
    bool IsEmpty() const;
    uint GetNumSubresources() const;
    uint GetNumSubresourcesPerPlane() const;

    uint myFirstMipLevel;
    uint myNumMipLevels;
    uint myFirstArrayIndex;
    uint myNumArrayIndices;
    uint myFirstPlane;
    uint myNumPlanes;
  };
//---------------------------------------------------------------------------//
  struct SubresourceIterator
  {
    explicit SubresourceIterator(const SubresourceRange& aRange);
    SubresourceIterator(const SubresourceRange& aRange, SubresourceLocation aLocation);

    const SubresourceLocation& operator*() const { return myCurrentLocation; }
    const SubresourceLocation* operator->() const { return &myCurrentLocation; }
    const SubresourceLocation& operator++();
    bool operator==(const SubresourceIterator& anOther) const;
    bool operator!=(const SubresourceIterator& anOther) const;

  private:
    bool IsEnd() const;

    SubresourceRange myRange;
    SubresourceLocation myCurrentLocation;
    SubresourceLocation myEndLocation;
  };
//---------------------------------------------------------------------------//
  struct TextureRegion
  {
    TextureRegion()
      : myPos(glm::uvec3(0))
      , mySize(glm::uvec3(UINT_MAX))
    {}

    TextureRegion(glm::uvec3 aPos, glm::uvec3 aSize)
      : myPos(std::move(aPos))
      , mySize(std::move(aSize))
    {}

    glm::uvec3 myPos;
    glm::uvec3 mySize;
  };
//---------------------------------------------------------------------------//
  struct TextureSubData
  {
    TextureSubData()
      : myData(nullptr)
      , myRowSizeBytes(0u)
      , mySliceSizeBytes(0u)
      , myTotalSizeBytes(0u)
    {}

    TextureSubData(const TextureProperties& someProperties);

    uint8* myData;
    uint64 myRowSizeBytes;
    uint64 mySliceSizeBytes;
    uint64 myTotalSizeBytes;
  };
//---------------------------------------------------------------------------//
  struct TextureData
  {
    TextureData() = default;
    TextureData(eastl::vector<uint8> someData, eastl::vector<TextureSubData> someSubDatas)
      : myData(std::move(someData))
      , mySubDatas(std::move(someSubDatas))
    {}
    
    static void ComputeRowPitchSizeAndBlockHeight(DataFormat aFormat, uint aWidth, uint aHeight, uint64& rowPitchSize, uint& aHeightBlocksOrPixels, uint aPlane = 0);

    eastl::vector<uint8> myData;
    eastl::vector<TextureSubData> mySubDatas;
  };
//---------------------------------------------------------------------------//
}