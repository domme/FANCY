#include "BinaryCache.h"
#include "GeometryData.h"

namespace Fancy {  namespace IO {
//---------------------------------------------------------------------------//
  bool BinaryCache::writeToCache(Rendering::Texture* aTexture, void* someData, uint32 aDataSize)
  {
    /*ASSERT(someData);
    ASSERT(aDataSize > 0u);

    Rendering::TextureDesc textureDesc = aTexture->getParameters();
    textureDesc.pPixelData = someData;
    textureDesc.uPixelDataSizeBytes = aDataSize;

    const String cacheFilePath = getCacheFilePathAbs(aTexture->getPath());
    PathService::createDirectoryTreeForPath(cacheFilePath);
    std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    if (!archive.good())
    {
      return false;
    }

    SerializerBinary binarySerializer(ESerializationMode::STORE, &archive);
    if (!binarySerializer.serialize(&textureDesc))
    {
      return false;
    }*/

    return true;
  }
//---------------------------------------------------------------------------//
  bool BinaryCache::writeToCache(Geometry::GeometryData* aGeometryData, void* someVertexData, uint32 aVertexDataSize, 
    void* someIndexData, uint32 anIndexDataSize)
  {
    //ASSERT(someVertexData);
    //ASSERT(someIndexData);

    //Geometry::GeometryDataDesc desc = aGeometryData->getDescription();
    //desc.myVertexData = someVertexData;
    //desc.myIndexData = someIndexData;

    //const String cacheFilePath = getCacheFilePathAbs(aGeometryData->getName());
    //PathService::createDirectoryTreeForPath(cacheFilePath);
    //std::fstream archive(cacheFilePath, std::ios::binary | std::ios::out);

    //if (!archive.good())
    //{
    //  return false;
    //}

    //SerializerBinary binarySerializer(ESerializationMode::STORE, &archive);
    //if (!binarySerializer.serialize(&desc))
    //{
    //  return false;
    //}

    return true;
  }
//---------------------------------------------------------------------------//
} }