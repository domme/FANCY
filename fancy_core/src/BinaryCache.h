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
      template<class T>
      static T* get(const ObjectName& _aName)
      {
        T* obj = T::getByName(_aName);

        if (obj == nullptr)
        {
          obj = loadFromCache<T>(_aName);
          if (obj)
          {
            T::registerWithName(_aName, obj);
          }
        }

        return obj;
      }
    //---------------------------------------------------------------------------//
      template <class T>
      static T* loadFromCache(const ObjectName& aName)
      {
        ASSERT_M(false, "Missing template specialization");
        return nullptr;
      }
    //---------------------------------------------------------------------------//
      template<> static Rendering::Texture* loadFromCache(const ObjectName& _aName)
      {
        const String cacheFilePath = getCacheFilePathAbs(_aName);
        std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

        if (!archive.good())
        {
          return nullptr;
        }

        SerializerBinary binarySerializer(ESerializationMode::LOAD, &archive);

        Rendering::TextureDesc textureDesc;
        if (!binarySerializer.serialize(&textureDesc))
        {
          return nullptr;
        }

        Rendering::Texture* texture = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);
        texture->create(textureDesc);

        FANCY_FREE(textureDesc.pPixelData, MemoryCategory::TEXTURES);

        return texture;
      }
    //---------------------------------------------------------------------------//
      static bool writeToCache(Rendering::Texture* aTexture, void* someData, uint32 aDataSize);
    //---------------------------------------------------------------------------//    
      static bool writeToCache(Geometry::GeometryData* aGeometryData, void* someVertexData, uint32 aVertexDataSize, void* someIndexData, uint32 anIndexDataSize);
    //---------------------------------------------------------------------------//          
    };
} }  // end of namespace Fancy::IO 


#endif  // INCLUDE_BINARYCACHE_H