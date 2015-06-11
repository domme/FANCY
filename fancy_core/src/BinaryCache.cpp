#include "BinaryCache.h"
#include "GeometryData.h"

namespace Fancy {  namespace IO {
//---------------------------------------------------------------------------//
  struct TextureHeader
  {
    uint32 myWidth;
    uint32 myHeight;
    uint32 myDepth;
    uint32 myFormat;
    uint32 myAccessFlags;
    uint32 myPixelDataSizeBytes;
    uint32 myNumMipmapLevels;
  };
//---------------------------------------------------------------------------//
  const String kBinaryCacheRoot = "Cache/";
  const String kBinaryCacheExtension = ".bin";
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  String BinaryCache::getCacheFilePathAbs(const ObjectName& aName)
  {
    return PathService::convertToAbsPath(kBinaryCacheRoot) 
      + aName.toString() + kBinaryCacheExtension;
  }
//---------------------------------------------------------------------------//  
  bool BinaryCache::write(Rendering::Texture* aTexture, void* someData, uint32 aDataSize)
  {
    ASSERT(someData);
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

    Rendering::TextureDesc desc = aTexture->getParameters();

    TextureHeader header;
    header.myWidth = desc.u16Width;
    header.myHeight = desc.u16Height;
    header.myDepth = desc.u16Depth;
    header.myAccessFlags = desc.uAccessFlags;
    header.myFormat = static_cast<uint32>(desc.eFormat);
    header.myNumMipmapLevels = desc.u8NumMipLevels;
    header.myPixelDataSizeBytes = aDataSize;
    archive.write(reinterpret_cast<const char*>(&header), sizeof(TextureHeader));
    archive.write(static_cast<const char*>(someData), aDataSize);

    return archive.good();
  }  
//---------------------------------------------------------------------------//
  bool BinaryCache::write(Geometry::GeometryData* aGeometryData, void* someVertexData, uint32 aVertexDataSize, 
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
  bool BinaryCache::load(Rendering::Texture** aTexture, const ObjectName& aName)
  {
    /*TextureHeader header;
    mySceneGraphStr->read((char*)&header, sizeof(TextureHeader));

    aTextureDesc->path = header.myPath.toString();
    aTextureDesc->u16Width = header.myWidth;
    aTextureDesc->u16Height = header.myHeight;
    aTextureDesc->u16Depth = header.myDepth;
    aTextureDesc->eFormat = static_cast<Rendering::DataFormat>(header.myFormat);
    aTextureDesc->u8NumMipLevels = header.myNumMipmapLevels;
    aTextureDesc->uAccessFlags = header.myAccessFlags;
    aTextureDesc->uPixelDataSizeBytes = header.myPixelDataSizeBytes;

    aTextureDesc->pPixelData = FANCY_ALLOCATE(header.myPixelDataSizeBytes, MemoryCategory::TEXTURES);
    ASSERT(aTextureDesc->pPixelData);

    mySceneGraphStr->read((char*)aTextureDesc->pPixelData, header.myPixelDataSizeBytes);*/

    return true;
  }
//---------------------------------------------------------------------------//
} }