#pragma once

#include "Serializer.h"
#include "Json/json.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  class JSONreader : public Serializer
  {
  public:
    JSONreader(const String& anArchivePath);
    virtual ~JSONreader() override;

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
    void loadHeader();

    RootHeader myHeader;
    Json::Value myDocumentVal;

    std::stack<Json::ArrayIndex> myArrayIndexStack;
    std::stack<Json::Value&> myTypeStack;
  };
//---------------------------------------------------------------------------//  
} }