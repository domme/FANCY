#pragma once

namespace Fancy { namespace IO {

  class Serializer;

//---------------------------------------------------------------------------//
  struct MetaTable
  {
    virtual ~MetaTable() {}
    virtual void* create() = 0;
    virtual void serialize(Serializer* aSerializer, void* anObject) = 0;
    virtual void destroy() = 0;
  };
//---------------------------------------------------------------------------//
  template<class T>
  struct MetaTableImpl : public MetaTable
  {
    virtual ~MetaTableImpl<T>() {}
    virtual void* create() override { return T::create(); }
    
    virtual void serialize(Serializer* aSerializer, void* anObject) override
    { 
      T* serializable = static_cast<T*>(anObject);
      serializable->serialize(aSerializer);
    }
    
    virtual void destroy() override { }

    static MetaTableImpl<T> ourMetaTable { MetaTableImpl<T>() };
  };
  
  // TODO: Special version of MetaTableImpl for dynamic and managed objects

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  enum class EBaseDataType
  {
    None,

    Int,
    Uint,
    Float,
    Char,
    String,
    CString,
    Array,
    Vector,
    Map,
    Serializable,
    SerializablePtr
  };
//---------------------------------------------------------------------------//
  struct DataType
  {
    DataType(EBaseDataType aBaseType, const void* aUserData = nullptr) 
      : myBaseType(aBaseType), myUserData(aUserData) {}

    EBaseDataType myBaseType;
    const void* myUserData;  // for Serializable types, this pointer hods a metatable-pointer which handles construction/destruction and serialization
  };
//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType
  {
    static DataType get()  // If this template is matched, we forgot some spezialization or the type is really not serializable...
    {
      ASSERT(false, "Datatype is not serializable");
      return DataType(EBaseDataType::None, nullptr);
    }
  };
//---------------------------------------------------------------------------//
#define DECLARE_DATATYPE(T, ET) \
  template<> \
  struct Get_DataType<T> \
  { \
    static DataType get() \
    { \
      return DataType(EBaseDataType::ET); \
    } \
  };

  // Base types (without meta-table)
  DECLARE_DATATYPE(int, Int);
  DECLARE_DATATYPE(uint, Uint);
  DECLARE_DATATYPE(float, Float);
  DECLARE_DATATYPE(char, Char);
  DECLARE_DATATYPE(const char*, CString);
  DECLARE_DATATYPE(std::string, String);
#undef DECLARE_DATATYPE

#define DECLARE_DATATYPE_SERIALIZABLE(T, ET) \
  template<> \
  struct Get_DataType<T> \
  { \
    static DataType get() \
    { \
      return DataType(EBaseDataType::ET, &MetaTableImpl<T>::ourMetaTable); \
    } \
  };

#define SERIALIZABLE(classT) DECLARE_DATATYPE_SERIALIZABLE(classT, Serializable)

//---------------------------------------------------------------------------//

} }  // end of namespace Fancy::IO
