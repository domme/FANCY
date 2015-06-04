#include "Serializer.h"
#include "RendererPrerequisites.h"

#include "SceneNodeComponent.h"
#include "SceneNode.h"
#include "SubModel.h"
#include "Mesh.h"
#include "Texture.h"
#include "MaterialPass.h"


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
  void SerializerJSON::beginType(const String& aTypeName, uint anInstanceHash)
  {
    Json::Value typeValue(Json::objectValue);
    typeValue["TypeName"] = aTypeName;

    if (anInstanceHash != 0x0)
      typeValue["InstanceHash"] = anInstanceHash;

    myTypeStack.push(typeValue);
  }
//---------------------------------------------------------------------------//
  Json::Value SerializerJSON::endType()
  {
    ASSERT_M(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");
    Json::Value val = myTypeStack.top();
    myTypeStack.pop();
    return val;
  }
//---------------------------------------------------------------------------//
  uint32 SerializerJSON::beginArray(const char* aName, uint32 aNumElements)
  {
    if (myMode == ESerializationMode::STORE)
    {
      ArrayDesc desc;
      desc.myName = aName;
      desc.myElementCount = aNumElements;
      myArrayStack.push(desc);

      myTypeStack.push(Json::Value(Json::arrayValue));

      return aNumElements;
    }
    else
    {
      // count stored elements and return count
      return 0;
    }
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::endArray()
  {
    if (myMode == ESerializationMode::STORE)
    {
      ASSERT_M(!myArrayStack.empty(), "Mismatching number of beginArray() / endArray() calls");
      ArrayDesc desc = myArrayStack.top();
      myArrayStack.pop();
      
      Json::Value& arrayVal = myTypeStack.top();
      myTypeStack.pop();
      ASSERT_M(!myTypeStack.empty(), "An array needs to be embedded in a type but there is none left");
      Json::Value& parentVal = myTypeStack.top();

      if (parentVal.type() == Json::arrayValue)  // multi-dimensional array
        parentVal.append(arrayVal);
      else
        parentVal[desc.myName] = arrayVal;
    }
    else
    {
      // TODO: Implement
    }
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, uint* aValue)
  {
    Json::Value val(*aValue);
    _store(aName, val);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, float* aValue)
  {
    Json::Value val(*aValue);
    _store(aName, val);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, String* aValue)
  {
    Json::Value val(*aValue);
    _store(aName, val);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, ObjectName* aValue)
  {
    _store(aName, (*aValue).toString());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, bool* aValue)
  {
    _store(aName, *aValue);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Scene::ELightType* aValue)
  {
    _store(aName, (uint32)(*aValue));
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Scene::SceneNode** aValue)
  {
    Scene::SceneNode* node = (*aValue);
    beginType(node->getTypeName(), node->getName());
    node->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Scene::SceneNodeComponentPtr* aValue)
  {
    Scene::SceneNodeComponentPtr& component = (*aValue);
    beginType(component->getTypeName(), 0);
    component->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
//  void SerializerJSON::store(const char* aName, std::vector<Scene::SceneNodeComponentPtr>* someValues)
//  {
//    Json::Value& currType = myTypeStack.top();
//    Json::Value array;
//
//    for (uint32 i = 0u; i < someValues->size(); ++i)
//    {
//      Scene::SceneNodeComponentPtr& sceneNodeComponent = (*someValues)[i];
//      Json::Value& item = beginType(sceneNodeComponent->getTypeName());
//      sceneNodeComponent->serialize(*this);
//      array.append(item);
//      endType();
//    }
//
//    currType[aName] = array;
//  }
////---------------------------------------------------------------------------//
//  void SerializerJSON::store(const char* aName, std::vector<Scene::SceneNodePtr>* someValues)
//  {
//
//  }
////---------------------------------------------------------------------------//
//  void SerializerJSON::store(const char* aName, Geometry::SubModelList* someValues)
//  {
//  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Geometry::Model** aValue)
  {
    Geometry::Model* val = (*aValue);
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Geometry::SubModel** aValue)
  {
    Geometry::SubModel* val = (*aValue);
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Geometry::Mesh** aValue)
  {
     Geometry::Mesh* val = (*aValue);
     _store(aName, val->getName().toString());

    // beginType(val->getTypeName(), val->getName());
    // val->serialize(*this);
    // _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::Material** aValue)
  {
    Rendering::Material* val = (*aValue);
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::MaterialPass** aValue)
  {
    Rendering::MaterialPass* val = *aValue;
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::MaterialPassInstance** aValue)
  {
    Rendering::MaterialPassInstance* val = *aValue;
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    _store(aName, endType());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::FillMode* aValue)
  {
    _store(aName, (uint32)*aValue);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::CullMode* aValue)
  {
    _store(aName, (uint32)* aValue);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::WindingOrder* aValue)
  {
    _store(aName, (uint32)* aValue);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::BlendState** aValue)
  {
    Rendering::BlendState* blendState = *aValue;
    _store(aName, blendState->getName().toString());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::DepthStencilState** aValue)
  {
    Rendering::DepthStencilState* dsstate = *aValue;
    _store(aName, dsstate->getName().toString());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::GpuProgram** aValue)
  {
    Rendering::GpuProgram* gpuProgram = *aValue;
    _store(aName, gpuProgram->getName().toString());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, Rendering::Texture** aValue)
  {
    Rendering::Texture* texture = *aValue;
    _store(aName, texture->getPath());
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, glm::quat* aValue)
  {
    Json::Value matVal(Json::arrayValue);
    for (uint32 y = 0; y < (*aValue).length(); ++y)
    {
      matVal.append((*aValue)[y]);
    }

    _store(aName, matVal);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, glm::mat3* aValue)
  {
    Json::Value matVal(Json::arrayValue);
    for (uint32 y = 0; y < (*aValue).length(); ++y)
    {
      for (uint32 x = 0; x < (*aValue)[y].length(); ++x)
      {
        matVal.append((*aValue)[x][y]);
      }
    }

    _store(aName, matVal);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, glm::mat4* aValue)
  {
    Json::Value matVal(Json::arrayValue);
    for (uint32 y = 0; y < (*aValue).length(); ++y)
    {
      for (uint32 x = 0; x < (*aValue)[y].length(); ++x)
      {
        matVal.append((*aValue)[x][y]);
      }
    }

    _store(aName, matVal);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, glm::vec3* aValue)
  {
    Json::Value matVal(Json::arrayValue);
    for (uint32 y = 0; y < (*aValue).length(); ++y)
    {
      matVal.append((*aValue)[y]);
    }

    _store(aName, matVal);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::store(const char* aName, glm::vec4* aValue)
  {
    Json::Value matVal(Json::arrayValue);
    for (uint32 y = 0; y < (*aValue).length(); ++y)
    {
      matVal.append((*aValue)[y]);
    }

    _store(aName, matVal);
  }
//---------------------------------------------------------------------------//
  void SerializerJSON::_store(const char* aName, const Json::Value& aValue)
  {
    Json::Value& currType = myTypeStack.top();
    if (currType.type() == Json::objectValue)
    {
      ASSERT(aName != nullptr);
      currType[aName] = aValue;
    }
    else
    {
      currType.append(aValue);
    }
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO

