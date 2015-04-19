#ifndef INCLUDE_BINARYCACHE_H
#define INCLUDE_BINARYCACHE_H

#include "FancyCorePrerequisites.h"
#include "PathService.h"
#include "Texture.h"
#include "Serializer.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
    const String kBinaryCacheRoot = "Cache/";
    const String kBinaryCacheExtension = ".bin";
//---------------------------------------------------------------------------//
    class BinaryCache
    {
    public:
      static String getCacheFilePathAbs(const String& _aResourcePath)
      {
        return PathService::convertToAbsPath(kBinaryCacheRoot) + PathService::toRelPath(_aResourcePath) + kBinaryCacheExtension;
      }

      static bool get(Rendering::Texture** _aTexture, const String& _aTexturePath)
      {
        const String aCachePathAbs = getCacheFilePathAbs(_aTexturePath);

        // TODO: Check file-existance, compare write-times of original and cache file

        SerializerBinary binarySerializer(ESerializationMode::LOAD);
        return binarySerializer.serialize(_aTexture, nullptr, 0u, aCachePathAbs);
      }

      static bool update(Rendering::Texture** _aTexture, const void* _aData, uint32 _aDataSize, const String& _aTexturePath)
      {
        const String aCachePathAbs = getCacheFilePathAbs(_aTexturePath);

        PathService::createDirectoryTreeForPath(aCachePathAbs);

        SerializerBinary binarySerializer (ESerializationMode::STORE);
        return binarySerializer.serialize(_aTexture, _aData, _aDataSize, aCachePathAbs);
      }
    };
//---------------------------------------------------------------------------//    
} }  // end of namespace Fancy::IO 


#endif  // INCLUDE_BINARYCACHE_H