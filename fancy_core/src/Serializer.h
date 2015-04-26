#ifndef INCLUDE_SERIALIZER_H
#define INCLUDE_SERIALIZER_H

#include "FancyCorePrerequisites.h"
#include <fstream>
#include "PathService.h"
#include "Texture.h"


namespace Fancy{namespace Scene{
  class Scene;
  class SceneNode;
}}

namespace Fancy { namespace IO {
    enum class ESerializationMode
    {
      STORE,
      LOAD
    };

    struct TextureHeader
    {
      ShortStringDesc myPath;
      uint32 myWidth;
      uint32 myHeight;
      uint32 myDepth;
      uint32 myFormat;
      uint32 myAccessFlags;
      uint32 myPixelDataSizeBytes;
      uint32 myNumMipmapLevels;
    };

//---------------------------------------------------------------------------//
    /*class Serializer
    {
      public:
        virtual bool serialize(Scene::SceneNode** someSceneNode, const String& someSerializePath) = 0;
    }; */
//---------------------------------------------------------------------------//
    class SerializerBinary // : public Serializer
    {
    public:
      SerializerBinary(ESerializationMode _aMode) { myMode = _aMode; }
    //---------------------------------------------------------------------------//      
      template<class T>
      bool serialize(T* anObject, std::fstream& aStream)
      {
        ASSERT(aStream.good());
        ASSERT(anObject);

        if (myMode == ESerializationMode::STORE)
        {
          return store(anObject, aStream);
        }
        else
        {
          return load(anObject, aStream);
        }
      }
    //---------------------------------------------------------------------------//
    private:

      template<class T>
      bool load(T* anObject, std::fstream& aStream)
      {
        ASSERT_M(false, "Missing template specialization");
      }
    //---------------------------------------------------------------------------//
      template<class T>
      bool store(T* anObject, std::fstream& aStream)
      {
        ASSERT_M(false, "Missing template specialization");
      }
    //---------------------------------------------------------------------------//
      template<>
      bool load(Rendering::TextureDesc* aTextureDesc, std::fstream& aStream)
      {
        TextureHeader header;
        aStream.read((char*)&header, sizeof(TextureHeader));

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

        aStream.read((char*)aTextureDesc->pPixelData, header.myPixelDataSizeBytes);

        return true;
      }
    //---------------------------------------------------------------------------//
      template<>
      bool store(Rendering::TextureDesc* aTextureDesc, std::fstream& aStream)
      {
        TextureHeader header;
        header.myPath = aTextureDesc->path;
        header.myWidth = aTextureDesc->u16Width;
        header.myHeight = aTextureDesc->u16Height;
        header.myDepth = aTextureDesc->u16Depth;
        header.myAccessFlags = aTextureDesc->uAccessFlags;
        header.myFormat = static_cast<uint32>(aTextureDesc->eFormat);
        header.myNumMipmapLevels = aTextureDesc->u8NumMipLevels;
        header.myPixelDataSizeBytes = aTextureDesc->uPixelDataSizeBytes;
        aStream.write(reinterpret_cast<const char*>(&header), sizeof(TextureHeader));
        aStream.write(static_cast<const char*>(aTextureDesc->pPixelData), aTextureDesc->uPixelDataSizeBytes);

        return aStream.good();
      }
    //---------------------------------------------------------------------------//
    private:
      ESerializationMode myMode;
    };

  } } // end of namespace Fancy::IO 

//BLOB-Serialization
  /*inline bool SerializerBinary::seek(uint32 _aHash, TocEntry& _anEntry)
  {
    for (uint32 i = 0u; i < myToc.size(); ++i)
    {
      if (myToc[i].myHash == _aHash)
      {
        _anEntry = myToc[i];
        return true;
      }
    }

    return false;
  }*/

//inline bool SerializerBinary::beginSerialization(const String& _aBlobPath, ESerializationMode _aMode)
//{
//  ASSERT_M(!myStream.good(), "Called beginSerialization() before endSerialization()");
//
//  myToc.clear();
//  myMode = _aMode;
//  String aPathAbs = PathService::convertToAbsPath(_aBlobPath);
//  myStream.open(aPathAbs, std::ios::in | std::ios::out | std::ios::binary);
//  ASSERT(myStream.good());
//
//  if (_aMode == ESerializationMode::STORE)
//  {
//    // reserve room for the header
//    Header header;
//    myStream.write(reinterpret_cast<const char*>(&header), sizeof(Header));
//  }
//  else
//  {
//    Header header;
//    myStream.read((char*)&header, sizeof(Header));
//
//    uint32 currReadPointer = myStream.tellg();
//    myStream.seekg(header.myTocOffset);
//    myStream.read((char*)&myToc, header.myTocSize);
//  }
//
//  return myStream.good();
//}
//
////---------------------------------------------------------------------------//
//inline bool SerializerBinary::endSerialization()
//{
//  ASSERT_M(myStream.good(), "Called endSerializtation() without beginSerialization()");
//
//  if (myMode == ESerializationMode::STORE)
//  {
//    // Construct the final header and override the reserved space in the beginning
//    Header header;
//    header.myTocOffset = myStream.tellp();
//    header.myTocSize = sizeof(myToc);
//    myStream.write((const char*)&myToc, sizeof(myToc));
//    myStream.seekp(0);
//    myStream.write((const char*)&header, sizeof(Header));
//  }
//
//  myStream.close();
//  return !myStream.good();
//}
//---------------------------------------------------------------------------//
#endif  // INCLUDE_FILEREADER_H