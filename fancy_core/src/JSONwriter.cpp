#include "JSONwriter.h"
#include "StringUtil.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  JSONwriter::JSONwriter(const String& anArchivePath) : Serializer(ESerializationMode::STORE)
  {
    uint32 archiveFlags = 0u;

    archiveFlags |= std::ios::out;

    String archivePath = anArchivePath + ".json";
    myArchive.open(archivePath, archiveFlags);

    myHeader = RootHeader();
    myHeader.myMeshes = Json::Value(Json::arrayValue);
    myHeader.myTextures = Json::Value(Json::arrayValue);
    myHeader.myGpuPrograms = Json::Value(Json::arrayValue);
    myHeader.myGpuProgramPipelines = Json::Value(Json::arrayValue);
    myHeader.myMaterials = Json::Value(Json::arrayValue);
    myHeader.myMaterialPassInstances = Json::Value(Json::arrayValue);
    myHeader.mySubModels = Json::Value(Json::arrayValue);

    myHeader.myMaterialPasses = Json::Value(Json::arrayValue);
    myHeader.myModels = Json::Value(Json::arrayValue);
    
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

    case EBaseDataType::Uint32:
    {
      currJsonVal = *static_cast<uint32*>(anObject);
    } break;

    case EBaseDataType::Uint8:
    {
      uint8 val8 = *static_cast<uint8*>(anObject);
      currJsonVal = static_cast<uint32>(val8);
    } break;

    case EBaseDataType::Uint16:
    {
      uint16 val16 = *static_cast<uint16*>(anObject);
      currJsonVal = static_cast<uint32>(val16);
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
    std::pair<ObjectName, Json::Value*> typeNameToVal[] = {
      { _N(Mesh), &myHeader.myMeshes },
      { _N(Texture), &myHeader.myTextures },
      { _N(GpuProgram), &myHeader.myGpuPrograms },
      { _N(GpuProgramPipeline), &myHeader.myGpuProgramPipelines },
      { _N(MaterialPass), &myHeader.myMaterialPasses },
      { _N(MaterialPassInstance), &myHeader.myMaterialPassInstances },
      { _N(Material), &myHeader.myMaterials },
      { _N(SubModel), &myHeader.mySubModels },
      { _N(Model), &myHeader.myModels },
    };

    for (uint32 i = 0u; i < ARRAY_LENGTH(typeNameToVal); ++i)
    {
      if (aTypeName == typeNameToVal[i].first)
        typeNameToVal[i].second->append(aResourceDescVal);
    }

    myHeader.myResourceDependencies.push_back(aHash);
  }
//---------------------------------------------------------------------------//
  bool JSONwriter::HasResourceDependency(uint64 aKey)
  {
    for (uint64 storedHash : myHeader.myResourceDependencies)
      if (storedHash == aKey)
        return true;

    return false;
  }
//---------------------------------------------------------------------------//
  void JSONwriter::storeHeader(Json::Value& aValue) const
  {
    aValue["myVersion"] = myHeader.myVersion;
    
    aValue["myMeshes"] = myHeader.myMeshes;
    aValue["myTextures"] = myHeader.myTextures;
    aValue["myGpuPrograms"] = myHeader.myGpuPrograms;
    aValue["myGpuProgramPipelines"] = myHeader.myGpuProgramPipelines;
    aValue["myMaterialPasses"] = myHeader.myMaterialPasses;
    aValue["myMaterialPassInstances"] = myHeader.myMaterialPassInstances;
    aValue["myMaterials"] = myHeader.myMaterials;
    aValue["mySubModels"] = myHeader.mySubModels;
    aValue["myModels"] = myHeader.myModels;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO