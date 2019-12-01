#pragma once

#include "FancyCoreDefines.h"
#include "MathIncludes.h"
#include "DynamicArray.h"

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

    bool operator==(const SubresourceRange& anOther) const;

    SubresourceIterator Begin() const;
    SubresourceIterator End() const;
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
      , myPixelSizeBytes(0u)
      , myRowSizeBytes(0u)
      , mySliceSizeBytes(0u)
      , myTotalSizeBytes(0u)
    {}

    TextureSubData(const TextureProperties& someProperties);

    uint8* myData;
    uint64 myPixelSizeBytes;
    uint64 myRowSizeBytes;
    uint64 mySliceSizeBytes;
    uint64 myTotalSizeBytes;
  };
//---------------------------------------------------------------------------//
  struct TextureData
  {
    TextureData() = default;
    TextureData(DynamicArray<uint8> someData, DynamicArray<TextureSubData> someSubDatas)
      : myData(std::move(someData))
      , mySubDatas(std::move(someSubDatas))
    {}
    DynamicArray<uint8> myData;
    DynamicArray<TextureSubData> mySubDatas;
  };
//---------------------------------------------------------------------------//
}