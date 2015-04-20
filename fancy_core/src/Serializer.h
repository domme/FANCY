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

//---------------------------------------------------------------------------//
    class Serializer
    {
      public:
        virtual bool serialize(Scene::SceneNode** someSceneNode, const String& someSerializePath) = 0;
    };
//---------------------------------------------------------------------------//
    class SerializerBinary : public Serializer
    {
    public:
      SerializerBinary(ESerializationMode _aMode) { myMode = _aMode; }
      
      virtual bool serialize(Scene::SceneNode** someSceneNode, const String& someSerializePath) override;
      bool serialize(Rendering::Texture** _aTexture, const void* _aData, uint32 _aDataSize, const String& _aSerializePath);
      
    private:
      bool store(Rendering::Texture** _aTexture, const void* _aData, uint32 _aDataSize, const String& _aSerializePath);
      bool load(Rendering::Texture** _aTexture, const String& _aSerializePath);
      bool store(Scene::SceneNode** someSceneNode, const String& someSerializePath);

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