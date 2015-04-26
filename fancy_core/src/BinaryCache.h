#ifndef INCLUDE_BINARYCACHE_H
#define INCLUDE_BINARYCACHE_H

#include "FancyCorePrerequisites.h"
#include "PathService.h"
#include "Texture.h"
#include "Serializer.h"
#include "StringUtil.h"

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
      template <class T>
      static bool writeToCache(T* anObject, void* someData, uint32 aDataSize)
      {
        ASSERT_M(false, "Missing template specialization");
        return false;
      }
      //---------------------------------------------------------------------------//
      template<>
      static Rendering::Texture* loadFromCache(const ObjectName& _aName)
      {
        const String cacheFilePath = getCacheFilePathAbs(_aName);
        std::fstream archive(cacheFilePath, std::ios::binary | std::ios::in);

        if (!archive.good())
        {
          return nullptr;
        }

        SerializerBinary binarySerializer(ESerializationMode::LOAD);

        Rendering::TextureDesc textureDesc;
        if (!binarySerializer.serialize(&textureDesc, archive))
        {
          return nullptr;
        }

        Rendering::Texture* texture = FANCY_NEW(Rendering::Texture, MemoryCategory::TEXTURES);
        texture->create(textureDesc);

        FANCY_FREE(textureDesc.pPixelData, MemoryCategory::TEXTURES);

        return texture;
      }
      //---------------------------------------------------------------------------//
      template<>
      static bool writeToCache(Rendering::Texture* aTexture, void* someData, uint32 aDataSize)
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

        SerializerBinary binarySerializer(ESerializationMode::STORE);
        if (!binarySerializer.serialize(&textureDesc, archive))
        {
          return false;
        }

        return true;
      }
      //---------------------------------------------------------------------------//    
    };
} }  // end of namespace Fancy::IO 


#endif  // INCLUDE_BINARYCACHE_H