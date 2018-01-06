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

    const uint myVersion = 0;

  protected:

    struct RootHeader
    {
      uint myVersion;
      
      Json::Value myResources;
      std::vector<uint64> myStoredResources;
    };

    bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

    void beginName(const char* aName, bool anIsArray) override;
    void endName() override;

    // New resourceDesc-based dependency system:
    void AddResourceDependency(const ObjectName& aTypeName, const Json::Value& aResourceDescVal, uint64 aHash);
    bool HasResourceDependency(uint64 aHash);
    
    void storeHeader(Json::Value& aValue) const;

    RootHeader myHeader;
    Json::Value myCurrentEndType;

    std::stack<const char*> myNameStack;
    std::stack<Json::Value> myTypeStack;
    Json::StyledStreamWriter myJsonWriter;
  };
//---------------------------------------------------------------------------//  
} }