#ifndef INCLUDE_SERIALIZER_H
#define INCLUDE_SERIALIZER_H

#include "FancyCorePrerequisites.h"
#include <fstream>
#include "FixedArray.h"

#include "Json/json.h"
#include "Serializable.h"

namespace Fancy { namespace IO {
  //---------------------------------------------------------------------------//
    enum class ESerializationMode
    {
      STORE,
      LOAD
    };
  //---------------------------------------------------------------------------//
    struct TypeInfo
    {
      String myTypeName;
      uint   myInstanceHash;
    };
  //---------------------------------------------------------------------------//
    struct TocEntry
    {
      TypeInfo myTypeInfo;
      uint32 myArchivePosition;
    };
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    class Serializer
    {
      public:
        Serializer(ESerializationMode aMode);
        virtual ~Serializer();

        ESerializationMode getMode() { return myMode; }

        template<class T> void serialize(T& anObject, const char* aName = nullptr)
        {
          typename std::is_enum<T>::type isEnum;
          _serialize(anObject, isEnum, aName);
        }

        template<class T> void _serialize(T& anObject, std::true_type anEnum, const char* aName = nullptr)
        {
          serializeImpl(DataType(EBaseDataType::Uint32), &anObject, aName);
        }

        template<class T> void _serialize(T& anObject, std::false_type anEnum, const char* aName = nullptr)
        {
          DataType dataType = Get_DataType<T>::get();
          serializeImpl(dataType, &anObject, aName);
        }

        template<class T> void serialize(T* anObject, const char* aName = nullptr)
        {
          serializeImpl(Get_DataType<T>::get(), anObject, aName);
        }

        template<class T, uint N> void serializeArray(T(&anObject)[N], const char* aName = nullptr)
        {
          serializeImpl(Get_DataTypeBuiltinArray<T, N>::get(), anObject, aName);
        }

    protected:

      virtual bool serializeImpl(DataType aDataType, void* anObject, const char* aName) = 0;

      virtual void beginName(const char* aName, bool anIsArray) = 0;
      virtual void endName() = 0;

      ESerializationMode myMode;
      std::fstream myArchive;
    };
  //---------------------------------------------------------------------------//
  //---------------------------------------------------------------------------//
    class JSONwriter : public Serializer
    {
    public:
      JSONwriter(const String& anArchivePath);
      virtual ~JSONwriter() override;
      
      const uint32 myVersion = 0;

    protected:
      struct ArrayDesc
      {
        String myName;
        uint32 myElementCount;
      };

      struct RootHeader
      {
        RootHeader() : myVersion(0u),
          myModels(Json::objectValue),
          mySubModels(Json::objectValue),
          myMaterials(Json::objectValue),
          myMaterialPasses(Json::objectValue) {}

        uint32 myVersion;

        Json::Value myModels;
        Json::Value mySubModels;
        Json::Value myMaterials;
        Json::Value myMaterialPasses;
      };

      virtual bool serializeImpl(DataType aDataType, void* anObject, const char* aName) override;

      virtual void beginName(const char* aName, bool anIsArray) override;
      virtual void endName() override;

      bool isStoredManaged(const ObjectName& aName, const Json::Value& aValue);
      void storeHeader(Json::Value& aValue);

      RootHeader myHeader;
      Json::Value myCurrentEndType;
      
      std::stack<const char*> myNameStack;
      std::stack<Json::Value> myTypeStack;
      Json::StyledStreamWriter myJsonWriter;
    };

  //---------------------------------------------------------------------------//
  } } // end of namespace Fancy::IO 
  
#endif  // INCLUDE_FILEREADER_H