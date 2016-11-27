#pragma once

#include "Serializer.h"
#include "Json/json.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class JSONwriter : public Serializer
  {
  public:
    JSONwriter(const String& anArchivePath);
    virtual ~JSONwriter() override;

    const uint32 myVersion = 0;

    void SerializeDescription(DescriptionBase* aDescription) override;

  protected:

    struct RootHeader
    {
      uint32 myVersion;
      
      Json::Value myMaterialPasses;
      Json::Value myMaterials;
      Json::Value myMaterialPassInstances;
      Json::Value myMeshes;
      Json::Value mySubModels;
      Json::Value myModels;

      std::vector<ObjectName> myStoredManagedObjects;
      
      Json::Value myTextures;
      Json::Value myGpuPrograms;
      Json::Value myGpuProgramPipelines;

      std::vector<uint64> myResourceDependencies;
    };

    virtual bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

    virtual void beginName(const char* aName, bool anIsArray) override;
    virtual void endName() override;

    // New resourceDesc-based dependency system:
    void AddResourceDependency(const ObjectName& aTypeName, const Json::Value& aResourceDescVal, uint64 aHash);
    bool HasResourceDependency(uint64 aHash);
    
    void appendResource(const ObjectName& aTypeName, const Json::Value& aResourceValue);
    bool isManagedObjectStored(const ObjectName& aName);
    void storeHeader(Json::Value& aValue);

    RootHeader myHeader;
    Json::Value myCurrentEndType;

    std::stack<const char*> myNameStack;
    std::stack<Json::Value> myTypeStack;
    Json::StyledStreamWriter myJsonWriter;
  };
//---------------------------------------------------------------------------//  
} }