#ifndef INCLUDE_SERIALIZER_H
#define INCLUDE_SERIALIZER_H

#include "FancyCorePrerequisites.h"
#include <fstream>
#include "FixedArray.h"

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

        template<class T> void serialize(T* anObject, const char* aName = nullptr)
        {
          DataType dataType = Get_DataType<T>::get();
          serializeImpl(dataType, anObject, aName);
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
    

  //---------------------------------------------------------------------------//
  } } // end of namespace Fancy::IO 
  
#endif  // INCLUDE_FILEREADER_H