#ifndef INCLUDE_BINARYCACHE_H
#define INCLUDE_BINARYCACHE_H

#include "FancyCorePrerequisites.h"
#include "PathService.h"
#include "Texture.h"
#include "Serializer.h"
#include "StringUtil.h"

namespace Fancy { namespace Geometry {
  class GeometryData;
} }

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
    const String kBinaryCacheRoot = "Cache/";
    const String kBinaryCacheExtension = ".bin";
//---------------------------------------------------------------------------//
    class BinaryCache
    {
    public:
    //---------------------------------------------------------------------------//
      static String getCacheFilePathAbs(const ObjectName& aName)
      {
        return PathService::convertToAbsPath(kBinaryCacheRoot) + StringUtil::toString(aName.getHash()) + kBinaryCacheExtension;
      }
    //---------------------------------------------------------------------------//
      void loadFromCache(Rendering::Texture** aTexture, const ObjectName& aName)
      {
        const String cacheFilePath = getCacheFilePathAbs(aName);
        std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

        if (!archive.good())
        {
          return;
        }

        /*SerializerBinary binarySerializer(ESerializationMode::LOAD, &archive);

        Rendering::TextureDesc textureDesc;
        if (!binarySerializer.serialize(&textureDesc))
        {
          return nullptr;
        }

        Rendering::Texture* texture = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);
        texture->create(textureDesc);

        FANCY_FREE(textureDesc.pPixelData, MemoryCategory::TEXTURES);

        return texture;*/
      }
    //---------------------------------------------------------------------------//
      static bool writeToCache(Rendering::Texture* aTexture, void* someData, uint32 aDataSize);
    //---------------------------------------------------------------------------//    
      static bool writeToCache(Geometry::GeometryData* aGeometryData, void* someVertexData, uint32 aVertexDataSize, void* someIndexData, uint32 anIndexDataSize);
    //---------------------------------------------------------------------------//      

     /* template<> void store(Rendering::TextureDesc* aTextureDesc, std::false_type isFundamental)
      {
        
      }


      template<> void load(Rendering::TextureDesc* aTextureDesc, std::false_type isFundamental)
      {
        TextureHeader header;
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

        mySceneGraphStr->read((char*)aTextureDesc->pPixelData, header.myPixelDataSizeBytes);
      }*/



    };
} }  // end of namespace Fancy::IO 


#endif  // INCLUDE_BINARYCACHE_H