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

  protected:

    struct RootHeader
    {
      uint32 myVersion;
      Json::Value myManagedObjects;
      std::vector<ObjectName> myStoredManagedObjects;
    };

    virtual bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

    virtual void beginName(const char* aName, bool anIsArray) override;
    virtual void endName() override;

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