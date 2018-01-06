#include "JSONwriter.h"
#include "StringUtil.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  JSONwriter::JSONwriter(const String& anArchivePath) : Serializer(ESerializationMode::STORE)
  {
    uint archiveFlags = 0u;

    archiveFlags |= std::ios::out;

    String archivePath = anArchivePath + ".json";
    myArchive.open(archivePath, archiveFlags);

    myHeader = RootHeader();
    myHeader.myResources = Json::Value(Json::arrayValue);
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
    case EBaseDataType::StructOrClass:
    {
      MetaTableStructOrClass* metaTable = static_cast<MetaTableStructOrClass*>(aDataType.myUserData);
      metaTable->Serialize(this, anObject);
    } break;
    case EBaseDataType::ResourcePtr:
    {
      MetaTableResource* metaTable = static_cast<MetaTableResource*>(aDataType.myUserData);
      if (!metaTable->IsValid(anObject))
      {
        currJsonVal = NULL;
        break;
      }

      SharedPtr<DescriptionBase> desc = metaTable->GetDescription(anObject);
      
      currJsonVal["Type"] = metaTable->GetTypeName(anObject);
      currJsonVal["Hash"] = desc->GetHash();
      
      Serialize(desc.get(), "Description");
    } break;
    case EBaseDataType::ResourceDesc:
    {
      DescriptionBase* desc = static_cast<DescriptionBase*>(anObject);

      const uint64 descHash = desc->GetHash();

      currJsonVal["Type"] = desc->GetTypeName().toString();
      currJsonVal["Hash"] = descHash;
      
      if (!HasResourceDependency(descHash))
      {
        myTypeStack.push(Json::Value(Json::objectValue));
        Json::Value& descVal = myTypeStack.top();
        descVal["Type"] = desc->GetTypeName().toString();
        descVal["Hash"] = descHash;
        desc->Serialize(this);
        AddResourceDependency(desc->GetTypeName(), descVal, descHash);
        myTypeStack.pop();
      }
    } break;
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
      uint64 instanceHash = metaTable->getHash(anObject);

      currJsonVal["Type"] = typeName;

      if (instanceHash != 0u)
        currJsonVal["Hash"] = instanceHash;
      
      metaTable->Serialize(this, anObject);
    } break;

    case EBaseDataType::Int:
    {
      currJsonVal = *static_cast<int*>(anObject);
    } break;

    case EBaseDataType::Uint:
    {
      currJsonVal = *static_cast<uint*>(anObject);
    } break;

    case EBaseDataType::Uint8:
    {
      uint8 val8 = *static_cast<uint8*>(anObject);
      currJsonVal = static_cast<uint>(val8);
    } break;

    case EBaseDataType::Uint16:
    {
      uint16 val16 = *static_cast<uint16*>(anObject);
      currJsonVal = static_cast<uint>(val16);
    } break;

    case EBaseDataType::Uint64:
    {
      uint64 val = *static_cast<uint64*>(anObject);
      currJsonVal = static_cast<uint64>(val);
    } break;

    case EBaseDataType::Float:
    {
      currJsonVal = *static_cast<float*>(anObject);
    } break;

    case EBaseDataType::Char:
    {
      currJsonVal = *static_cast<char*>(anObject);
    } break;

    case EBaseDataType::Bool:
    {
      currJsonVal = *static_cast<bool*>(anObject);
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
      ObjectName& name = *static_cast<ObjectName*>(anObject);
      currJsonVal = name.toString();
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
      for (uint i = 0u; i < static_cast<uint>(val.length()); ++i)
        jsonVal.append(val[i]);

      currJsonVal = jsonVal;
    } break;

    case EBaseDataType::Vector4:
    {
      const glm::vec4& val = *static_cast<const glm::vec4*>(anObject);
      Json::Value jsonVal(Json::arrayValue);
      for (uint i = 0u; i < static_cast<uint>(val.length()); ++i)
        jsonVal.append(val[i]);

      currJsonVal = jsonVal;
    } break;

    case EBaseDataType::Quaternion:
    {
      const glm::quat& val = *static_cast<const glm::quat*>(anObject);
      Json::Value jsonVal(Json::arrayValue);
      for (uint i = 0u; i < (uint) val.length(); ++i)
        jsonVal.append(val[i]);

      currJsonVal = jsonVal;
    } break;

    case EBaseDataType::Matrix3x3:
    {
      const glm::mat3& val = *static_cast<const glm::mat3*>(anObject);
      Json::Value jsonVal(Json::arrayValue);
      for (uint y = 0u; y < (uint) val.length(); ++y)
        for (uint x = 0u; x < (uint) val[y].length(); ++x)
          jsonVal.append(val[x][y]);

      currJsonVal = jsonVal;
    } break;

    case EBaseDataType::Matrix4x4:
    {
      const glm::mat4& val = *static_cast<const glm::mat4*>(anObject);
      Json::Value jsonVal(Json::arrayValue);
      for (uint y = 0u; y < (uint) val.length(); ++y)
        for (uint x = 0u; x < (uint) val[y].length(); ++x)
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
    ASSERT(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");
    ASSERT(!myNameStack.empty(), "Mismatching number of beginType() / endType() calls");

    myCurrentEndType = myTypeStack.top();
    myTypeStack.pop();

    const char* name = myNameStack.top();
    myNameStack.pop();

    if (!myTypeStack.empty())
    {
      Json::Value& parentVal = myTypeStack.top();
      const bool isArray = parentVal.type() == Json::arrayValue;

      if (isArray)
        parentVal.append(myCurrentEndType);
      else
        parentVal[name] = myCurrentEndType;
    }
  }
//---------------------------------------------------------------------------//
  void JSONwriter::AddResourceDependency(const ObjectName& aTypeName, const Json::Value& aResourceDescVal, uint64 aHash)
  {
    myHeader.myResources.append(aResourceDescVal);
    myHeader.myStoredResources.push_back(aHash);
  }
//---------------------------------------------------------------------------//
  bool JSONwriter::HasResourceDependency(uint64 aKey)
  {
    for (uint64 storedHash : myHeader.myStoredResources)
      if (storedHash == aKey)
        return true;

    return false;
  }
//---------------------------------------------------------------------------//
  void JSONwriter::storeHeader(Json::Value& aValue) const
  {
    aValue["myVersion"] = myHeader.myVersion;
    aValue["myResources"] = myHeader.myResources;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO