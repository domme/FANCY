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
  Serializer::Serializer(ESerializationMode aMode) :
    myMode(aMode) 
  {
    
  }
//---------------------------------------------------------------------------//
  Serializer::~Serializer()
  {
    if (myArchive.good())
    {
      myArchive.close();
    }
  }
//---------------------------------------------------------------------------//
  bool Serializer::serializeImpl(void* anObject, DataType aDataType, const char* aName)
  {
    switch (aDataType.myBaseType)
    {
      case EBaseDataType::Serializable:
        MetaTable* metaTable = static_cast<MetaTable*>(aDataType.myUserData);
        beginType(metaTable->getTypeName(anObject), metaTable->getInstanceName(anObject));
        metaTable->serialize(this, anObject);
        endType();
        return true;
      default:
        return false;
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  JSONwriter::JSONwriter(const String& anArchivePath) : Serializer(ESerializationMode::STORE)
  {
    uint32 archiveFlags = 0u;

    archiveFlags |= std::ios::out;

    String archivePath = anArchivePath + Internal::kArchiveExtensionJson;
    myArchive.open(archivePath, archiveFlags);

    myHeader = RootHeader();
    JSONwriter::beginType("Root", "");
  }
//---------------------------------------------------------------------------//
  JSONwriter::~JSONwriter()
  {
    JSONwriter::endType();
    
    storeHeader(myCurrentEndType);
    myJsonWriter.write(myArchive, myCurrentEndType);
  }
//---------------------------------------------------------------------------//
  bool JSONwriter::serializeImpl(void* anObject, DataType aDataType, const char* aName)
  {
    if (Serializer::serializeImpl(anObject, aDataType, aName))
      return true;

    bool handled = true;
    switch (aDataType.myBaseType)
    {
      case EBaseDataType::None: break;

      case EBaseDataType::Int:
      {
        Json::Value jsonVal(*static_cast<int*>(anObject));
        _store(aName, jsonVal);
      } break;
        
      case EBaseDataType::Uint: 
      {
        Json::Value jsonVal(*static_cast<uint*>(anObject));
        _store(aName, jsonVal);
      } break;

      case EBaseDataType::Float:
      {
        Json::Value jsonVal(*static_cast<uint*>(anObject));
        _store(aName, jsonVal);
      } break;

      case EBaseDataType::Char:
      {
        Json::Value jsonVal(*static_cast<char*>(anObject));
        _store(aName, jsonVal);
      } break;

      case EBaseDataType::String:
      {
        Json::Value jsonVal(*static_cast<String*>(anObject));
        _store(aName, jsonVal);
      } break;
        
      case EBaseDataType::CString:
      {
        Json::Value jsonVal(*static_cast<const char*>(anObject));
        _store(aName, jsonVal);
      } break;

      case EBaseDataType::Array: break;
      case EBaseDataType::Vector: break;
      case EBaseDataType::Map: break;

      case EBaseDataType::Vector3:
      {
        const glm::vec3& val = *static_cast<const glm::vec3*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint i = 0u; i < val.length(); ++i)
          jsonVal.append(val[i]);

        _store(aName, jsonVal);
      } break;

      case EBaseDataType::Vector4:
      {
        const glm::vec4& val = *static_cast<const glm::vec4*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint i = 0u; i < val.length(); ++i)
          jsonVal.append(val[i]);

        _store(aName, jsonVal);
      } break;

      case EBaseDataType::Matrix3x3: 
      {
        const glm::mat3& val = *static_cast<const glm::mat3*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint y = 0u; y < val.length(); ++y)
          for (uint x = 0u; x < val[y].length(); ++x)
          jsonVal.append(val[x][y]);

        _store(aName, jsonVal);
      } break;

      case EBaseDataType::Matrix4x4:
      {
        const glm::mat4& val = *static_cast<const glm::mat4*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint y = 0u; y < val.length(); ++y)
          for (uint x = 0u; x < val[y].length(); ++x)
            jsonVal.append(val[x][y]);

        _store(aName, jsonVal);
      } break;

      default: 
        handled = false;
        break;
    }

    return handled;
  }
//---------------------------------------------------------------------------//
  void JSONwriter::_store(const char* aName, const Json::Value& aValue)
  {
    Json::Value& currType = myTypeStack.top();
    if (currType.type() == Json::objectValue)
    {
      if (currType.type() == Json::objectValue)
      {
        ASSERT(aName != nullptr);
        currType[aName] = aValue;
      }
      else
      {
        currType.append(aValue);
      }
      ASSERT(aName != nullptr);
      currType[aName] = aValue;
    }
  }
//---------------------------------------------------------------------------//
  void JSONwriter::beginType(const String& aTypeName, const String& aName)
  {
    Json::Value typeValue(Json::objectValue);
    typeValue["Type"] = aTypeName;

    if (!aName.empty())
      typeValue["Name"] = aName;

    myTypeStack.push(typeValue);
  }
//---------------------------------------------------------------------------//
  void JSONwriter::endType()
  {
    ASSERT_M(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");
    myCurrentEndType = myTypeStack.top();
    myTypeStack.pop();
  }
//---------------------------------------------------------------------------//
  uint32 JSONwriter::beginArray(const char* aName, uint32 aNumElements)
  {
    ArrayDesc desc;
    desc.myName = aName;
    desc.myElementCount = aNumElements;
    myArrayStack.push(desc);

    myTypeStack.push(Json::Value(Json::arrayValue));

    return aNumElements;
  }
//---------------------------------------------------------------------------//
  void JSONwriter::endArray()
  {
    ASSERT_M(!myArrayStack.empty(), "Mismatching number of beginArray() / endArray() calls");
    ArrayDesc desc = myArrayStack.top();
    myArrayStack.pop();
      
    Json::Value arrayVal = myTypeStack.top();
    myTypeStack.pop();
    ASSERT_M(!myTypeStack.empty(), "An array needs to be embedded in a type but there is none left");
    Json::Value& parentVal = myTypeStack.top();

    if (parentVal.type() == Json::arrayValue)  // multi-dimensional array
      parentVal.append(arrayVal);
    else
      parentVal[desc.myName] = arrayVal;
  }
//---------------------------------------------------------------------------//
  void JSONwriter::storeHeader(Json::Value& aValue)
  {
    aValue["myVersion"] = myHeader.myVersion;
    aValue["myModels"] = myHeader.myModels;
    aValue["mySubModels"] = myHeader.mySubModels;
    aValue["myMaterials"] = myHeader.myMaterials;
    aValue["myMaterialPasses"] = myHeader.myMaterialPasses;
  }
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  bool JSONwriter::isStoredManaged(const ObjectName& aName, const Json::Value& aVal)
  {
    ASSERT(aVal.type() == Json::objectValue);
    for (Json::ValueConstIterator it = aVal.begin(); it != aVal.end(); ++it)
    {
      if (it.name() == aName.toString())
      {
        return true;
      }
    }

    return false;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO



#if defined USE_OLD_STORE_FUNCTIONS

void SerializerJSON::store(const char* aName, uint32* aValue)
{
  Json::Value val(*aValue);
  _store(aName, val);
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
  if (!node) {
    _store(aName, NULL);
    return;
  }

  beginType(node->getTypeName(), node->getName());
  node->serialize(*this);
  _store(aName, endType());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, std::shared_ptr<Scene::SceneNode>* aValue)
{
  Scene::SceneNode* node = aValue->get();
  store(aName, &node);
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Scene::SceneNodeComponentPtr* aValue)
{
  Scene::SceneNodeComponentPtr& component = (*aValue);
  beginType(component->getTypeName(), "");
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
  if (!val) {
    _store(aName, NULL);
    return;
  }

  if (!isStoredManaged(val->getName(), myHeader.myModels))
  {
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    myHeader.myModels[val->getName().toString()] = endType();
  }

  _store(aName, val->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Geometry::SubModel** aValue)
{
  Geometry::SubModel* val = (*aValue);
  if (!val) {
    _store(aName, NULL);
    return;
  }

  if (!isStoredManaged(val->getName(), myHeader.mySubModels))
  {
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    myHeader.mySubModels[val->getName().toString()] = endType();
  }

  _store(aName, val->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Geometry::Mesh** aValue)
{
  Geometry::Mesh* val = (*aValue);
  if (!val) {
    _store(aName, NULL);
    return;
  }

  _store(aName, val->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Rendering::Material** aValue)
{
  Rendering::Material* val = (*aValue);
  if (!val) {
    _store(aName, NULL);
    return;
  }

  if (!isStoredManaged(val->getName(), myHeader.myMaterials))
  {
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    myHeader.myMaterials[val->getName().toString()] = endType();
  }

  _store(aName, val->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Rendering::MaterialPass** aValue)
{
  Rendering::MaterialPass* val = *aValue;
  if (!val) {
    _store(aName, NULL);
    return;
  }

  if (!isStoredManaged(val->getName(), myHeader.myMaterialPasses))
  {
    beginType(val->getTypeName(), val->getName());
    val->serialize(*this);
    myHeader.myMaterialPasses[val->getName().toString()] = endType();
  }

  _store(aName, val->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Rendering::MaterialPassInstance** aValue)
{
  Rendering::MaterialPassInstance* val = *aValue;

  if (!val) {
    _store(aName, NULL);
    return;
  }

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

  if (!dsstate) {
    _store(aName, NULL);
    return;
  }

  _store(aName, dsstate->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Rendering::GpuProgram** aValue)
{
  Rendering::GpuProgram* gpuProgram = *aValue;

  if (!gpuProgram) {
    _store(aName, NULL);
    return;
  }

  _store(aName, gpuProgram->getName().toString());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Rendering::Texture** aValue)
{
  Rendering::Texture* texture = *aValue;

  if (!texture) {
    _store(aName, NULL);
    return;
  }

  _store(aName, texture->getPath());
}
//---------------------------------------------------------------------------//
void SerializerJSON::store(const char* aName, Rendering::TextureStorageEntry* aValue)
{
  beginType("TextureEntry", "");
  aValue->serialize(*this);
  _store(aName, endType());
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

#endif  // USE_OLD_STORE_FUNCTIONS
