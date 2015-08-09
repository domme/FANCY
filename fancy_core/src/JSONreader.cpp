#include "JSONreader.h"
#include "GpuProgram.h"

namespace Fancy { namespace IO {
  Json::Value nullVal = Json::Value(NULL);
//---------------------------------------------------------------------------//
  JSONreader::JSONreader(const String& anArchivePath) : Serializer(ESerializationMode::LOAD)
  {
    uint32 archiveFlags = 0u;

    archiveFlags |= std::ios::in;

    String archivePath = anArchivePath + ".json";
    myArchive.open(archivePath, archiveFlags);

    if (myArchive.good())
    {
      myArchive >> myDocumentVal;
    }

    myTypeStack.push(myDocumentVal);
    JSONreader::beginName("Root", false);
    loadHeader();
  }
//---------------------------------------------------------------------------//
  JSONreader::~JSONreader()
  {
    
  }
//---------------------------------------------------------------------------//
  bool JSONreader::serializeImpl(DataType aDataType, void* anObject, const char* aName)
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
      if (!metaTable->isValid(anObject))
      {
        currJsonVal = NULL;
        break;
      }

      String typeName = metaTable->getTypeName(anObject);
      String instanceName = metaTable->getInstanceName(anObject);

      currJsonVal["Type"] = typeName;

      if (!instanceName.empty())
        currJsonVal["Name"] = instanceName;

      const bool isManaged = metaTable->isManaged(anObject);
      if (isManaged)
      {
        String key = typeName + "_" + instanceName;
        if (!isManagedObjectStored(key))
        {
          metaTable->serialize(this, anObject);
          myHeader.myManagedObjects.append(currJsonVal);
          currJsonVal.clear();

          currJsonVal["Type"] = typeName;

          if (!instanceName.empty())
            currJsonVal["Name"] = instanceName;

          myHeader.myStoredManagedObjects.push_back(key);
        }
      }
      else
      {
        metaTable->serialize(this, anObject);
      }
    } break;

    case EBaseDataType::Int:
    {
      *static_cast<int*>(anObject) = currJsonVal.asInt();
    } break;

    case EBaseDataType::Uint:
    {
      *static_cast<uint*>(anObject) = currJsonVal.asUInt64();
    } break;

    case EBaseDataType::Uint32:
    {
      *static_cast<uint32*>(anObject) = currJsonVal.asUInt();
    } break;

    case EBaseDataType::Float:
    {
      *static_cast<float*>(anObject) = currJsonVal.asFloat();
    } break;

    case EBaseDataType::Char:
    {
      *static_cast<char*>(anObject) = currJsonVal.asUInt();
    } break;

    case EBaseDataType::Bool:
    {
      *static_cast<bool*>(anObject) = currJsonVal.asBool();
    } break;

    case EBaseDataType::String:
    {
      *static_cast<String*>(anObject) = currJsonVal.asString();
    } break;

    case EBaseDataType::CString:
    {
      *static_cast<const char**>(anObject) = currJsonVal.asCString();
    } break;

    case EBaseDataType::ObjectName:
    {
      *static_cast<ObjectName*>(anObject) = currJsonVal.asString();
    } break;

    case EBaseDataType::Array:
    {
      MetaTableArray* arrayVtable = reinterpret_cast<MetaTableArray*>(aDataType.myUserData);
      uint numElements = currJsonVal.size();
      arrayVtable->resize(anObject, numElements);
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
  void JSONreader::beginName(const char* aName, bool anIsArray)
  {
    Json::Value& parentVal = myTypeStack.top();

    Json::Value* newVal = nullptr;
    if (parentVal.isArray())
    {
      Json::ArrayIndex& index = myArrayIndexStack.top();
      Json::Value& val = parentVal[index];
      newVal = &val;
      ++index;
    }
    else
    {
      Json::Value& val = parentVal[aName];
      newVal = &val;
    }

    if (newVal->isArray())
      myArrayIndexStack.push(0);
  }
//---------------------------------------------------------------------------//
  void JSONreader::endName()
  {
    ASSERT_M(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");

    Json::Value& val = myTypeStack.top();
    if (val.isArray())
      myArrayIndexStack.pop();

    myTypeStack.pop();
  }
//---------------------------------------------------------------------------//
  void JSONreader::loadHeader()
  {
    Json::Value& rootVal = myTypeStack.top();
    myHeader.myVersion = rootVal["myVersion"].asUInt();
    myHeader.myManagedObjects = rootVal["myManagedResources"];

    for (Json::ValueIterator it = myHeader.myManagedObjects.begin(); it != myHeader.myManagedObjects.end(); ++it)
    {
      Json::Value& currVal = *it;
      myTypeStack.push(currVal);
      ObjectName typeName = currVal["Type"].asString();

      if (typeName == _N(GpuProgram))
      {
        Rendering::GpuProgram* program = nullptr;
        serialize(program);
      }

      myTypeStack.pop();
    }
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  bool JSONreader::isManagedObjectStored(const ObjectName& aName)
  {
    for (const ObjectName& storedName : myHeader.myStoredManagedObjects)
    {
      if (storedName == aName)
        return true;
    }

    return false;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO