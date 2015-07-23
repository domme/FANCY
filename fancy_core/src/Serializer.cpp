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

//---------------------------------------------------------------------------//
  JSONwriter::JSONwriter(const String& anArchivePath) : Serializer(ESerializationMode::STORE)
  {
    uint32 archiveFlags = 0u;

    archiveFlags |= std::ios::out;

    String archivePath = anArchivePath + Internal::kArchiveExtensionJson;
    myArchive.open(archivePath, archiveFlags);

    myHeader = RootHeader();
    JSONwriter::beginName("Root", false);
  }
//---------------------------------------------------------------------------//
  JSONwriter::~JSONwriter()
  {
    JSONwriter::endName();
    
    storeHeader(myCurrentEndType);
    myJsonWriter.write(myArchive, myCurrentEndType);
  }
//---------------------------------------------------------------------------//
  bool JSONwriter::serializeImpl(DataType aDataType, void* anObject, const char* aName)
  {
    beginName(aName, aDataType.myBaseType == EBaseDataType::Array);

    Json::Value& currJsonVal = myTypeStack.top();

    bool handled = true;
    switch (aDataType.myBaseType)
    {
      case EBaseDataType::Serializable:
      case EBaseDataType::SerializablePtr:
      {
        MetaTable* metaTable = static_cast<MetaTable*>(aDataType.myUserData);
        currJsonVal["Type"] = metaTable->getTypeName(anObject);

        if (!metaTable->getInstanceName(anObject).empty())
          currJsonVal["Name"] = metaTable->getInstanceName(anObject);
        
        metaTable->serialize(this, anObject);
      } break;
      
      case EBaseDataType::Int:
      {
        currJsonVal = *static_cast<int*>(anObject);
      } break;
        
      case EBaseDataType::Uint: 
      {
        currJsonVal = *static_cast<uint*>(anObject);
      } break;

      case EBaseDataType::Float:
      {
        currJsonVal = *static_cast<uint*>(anObject);
      } break;

      case EBaseDataType::Char:
      {
        currJsonVal = *static_cast<char*>(anObject);
      } break;

      case EBaseDataType::String:
      {
        currJsonVal = *static_cast<String*>(anObject);
      } break;
        
      case EBaseDataType::CString:
      {
        currJsonVal = *static_cast<const char*>(anObject);
      } break;

      case EBaseDataType::ObjectName:
      {
        currJsonVal = static_cast<ObjectName*>(anObject)->toString();
      } break;

      case EBaseDataType::Array:
      {
        MetaTableArray* arrayVtable = reinterpret_cast<MetaTableArray*>(aDataType.myUserData);
        uint numElements = arrayVtable->getSize(anObject);
        for (uint i = 0u; i < numElements; ++i)
          serializeImpl(arrayVtable->getElementDataType(), arrayVtable->getElement(anObject, i), nullptr);
      } break;

      case EBaseDataType::Map: break;

      case EBaseDataType::Vector3:
      {
        const glm::vec3& val = *static_cast<const glm::vec3*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint i = 0u; i < val.length(); ++i)
          jsonVal.append(val[i]);

        currJsonVal = jsonVal;
      } break;

      case EBaseDataType::Vector4:
      {
        const glm::vec4& val = *static_cast<const glm::vec4*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint i = 0u; i < val.length(); ++i)
          jsonVal.append(val[i]);

        currJsonVal = jsonVal;
      } break;

      case EBaseDataType::Quaternion:
      {
        const glm::quat& val = *static_cast<const glm::quat*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint i = 0u; i < val.length(); ++i)
          jsonVal.append(val[i]);

        currJsonVal = jsonVal;
      } break;

      case EBaseDataType::Matrix3x3: 
      {
        const glm::mat3& val = *static_cast<const glm::mat3*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint y = 0u; y < val.length(); ++y)
          for (uint x = 0u; x < val[y].length(); ++x)
          jsonVal.append(val[x][y]);

        currJsonVal = jsonVal;
      } break;

      case EBaseDataType::Matrix4x4:
      {
        const glm::mat4& val = *static_cast<const glm::mat4*>(anObject);
        Json::Value jsonVal(Json::arrayValue);
        for (uint y = 0u; y < val.length(); ++y)
          for (uint x = 0u; x < val[y].length(); ++x)
            jsonVal.append(val[x][y]);

        currJsonVal = jsonVal;
      } break;

      case EBaseDataType::None:
      default: 
        handled = false;
        break;
    }

    endName();

    return handled;
  }
//---------------------------------------------------------------------------//
  void JSONwriter::beginName(const char* aName, bool anIsArray)
  {
    myTypeStack.push(anIsArray ? Json::Value(Json::arrayValue) : Json::Value(Json::objectValue));
    myNameStack.push(aName);
  }
//---------------------------------------------------------------------------//
  void JSONwriter::endName()
  {
    ASSERT_M(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");
    ASSERT_M(!myNameStack.empty(), "Mismatching number of beginType() / endType() calls");

    myCurrentEndType = myTypeStack.top();
    myTypeStack.pop();

    const char* name = myNameStack.top();
    myNameStack.pop();

    if (!myTypeStack.empty())
    {
      Json::Value& parentVal = myTypeStack.top();
      const bool isArray = parentVal.type() == Json::arrayValue || name == nullptr;

      if (isArray)
        parentVal.append(myCurrentEndType);
      else
        parentVal[name] = myCurrentEndType;
    }
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
