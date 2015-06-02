#include "Serializer.h"
#include "SceneNodeComponent.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  namespace Internal {
    const String kArchiveExtensionJson = ".json";
    const String kArchiveExtensionBinary = ".bin";
  }
//---------------------------------------------------------------------------//

#if SERIALIZER_BINARY_SUPPORT
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
  void SerializerBinary::writeToArchive(Rendering::Texture* aTexture, void* someData, uint32 aDataSize, std::fstream* anArchive)
  {
    const Rendering::TextureDesc& desc = aTexture->getParameters();

    TextureHeader header;
    header.myPath = desc.path;
    header.myWidth = desc.u16Width;
    header.myHeight = desc.u16Height;
    header.myDepth = desc.u16Depth;
    header.myAccessFlags = desc.uAccessFlags;
    header.myFormat = static_cast<uint32>(desc.eFormat);
    header.myNumMipmapLevels = desc.u8NumMipLevels;
    header.myPixelDataSizeBytes = aDataSize;
    anArchive->write(reinterpret_cast<const char*>(&header), sizeof(TextureHeader));
    anArchive->write(static_cast<const char*>(someData), aDataSize);
  }
//---------------------------------------------------------------------------//
  void SerializerBinary::writeToArchive(Geometry::GeometryData* aGeometry, std::fstream* anArchive)
  {
  }
//---------------------------------------------------------------------------//
  void SerializerBinary::loadFromArchive(Rendering::Texture** aTexture, std::fstream* anArchive)
  {

  }
//---------------------------------------------------------------------------//
  void SerializerBinary::loadFromArchive(Geometry::GeometryData** aTexture, std::fstream* anArchive)
  {
  }
//---------------------------------------------------------------------------//
#endif


//---------------------------------------------------------------------------//
  Serializer::Serializer(ESerializationMode aMode) :
    myMode(aMode) 
  {
    
  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  SerializerJSON::SerializerJSON(ESerializationMode aMode, const String& anArchivePath)
    : Serializer::Serializer(aMode)
  {
    uint32 archiveFlags = 0u;

    if (aMode == ESerializationMode::LOAD)
      archiveFlags |= std::ios::in;
    else
      archiveFlags |= std::ios::out;

    String archivePath = anArchivePath + Internal::kArchiveExtensionJson;
    myArchive.open(archivePath, archiveFlags);
  }
//---------------------------------------------------------------------------//
  Json::Value& SerializerJSON::beginType(const String& aTypeName, uint anInstanceHash)
  {
    Json::Value& typeValue = beginType(aTypeName);
    typeValue["myInstanceHash"] = anInstanceHash;

    return typeValue;
  }
//---------------------------------------------------------------------------//
  Json::Value& SerializerJSON::beginType(const String& aTypeName)
  {
    Json::Value typeValue(Json::objectValue);
    typeValue = aTypeName;

    myTypeStack.push(typeValue);
    return myTypeStack.top();
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::endType()
  {
    ASSERT_M(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");
    myTypeStack.pop();
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, uint* aValue)
  {
    Json::Value& currType = myTypeStack.top();
    currType[aName] = *aValue;
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, float* aValue)
  {
    Json::Value& currType = myTypeStack.top();
    currType[aName] = *aValue;
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, String* aValue)
  {
    Json::Value& currType = myTypeStack.top();
    currType[aName] = aValue->c_str();
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, std::vector<Scene::SceneNodeComponentPtr>* someValues)
  {
    Json::Value& currType = myTypeStack.top();
    Json::Value array;

    for (uint32 i = 0u; i < someValues->size(); ++i)
    {
      Scene::SceneNodeComponentPtr& sceneNodeComponent = (*someValues)[i];
      Json::Value& item = beginType(sceneNodeComponent->getTypeName());
      sceneNodeComponent->serialize(*this);
      array.append(item);
      endType();
    }

    currType[aName] = array;
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, std::vector<Scene::SceneNodePtr>* someValues)
  {

  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, Geometry::Model** aValue)
  {

  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, Geometry::SubModel** aValue)
  {

  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, Geometry::SubModelList* someValues)
  {
  }

  //---------------------------------------------------------------------------//
  void SerializerJSON::store(const String& aName, Rendering::Material** aValue)
  {

  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

