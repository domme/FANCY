#pragma once

#include "ObjectName.h"

namespace Fancy { namespace IO {

  class Serializer;

//---------------------------------------------------------------------------//
  struct MetaTable
  {
    virtual ~MetaTable() {}
    virtual void* create() = 0;
    virtual String getTypeName(void* anObject) = 0;
    virtual String getInstanceName(void* anObject) = 0;
    virtual void serialize(Serializer* aSerializer, void* anObject) = 0;
    virtual void destroy() = 0;
  };
//---------------------------------------------------------------------------//
  template<class T>
  struct MetaTableImpl : public MetaTable
  {
    virtual ~MetaTableImpl<T>() {}
    virtual void* create() override { return nullptr; }

    virtual String getTypeName(void* anObject) override
    { 
      T* serializable = static_cast<T*>(anObject);
      return serializable->getTypeName();
    }

    virtual String getInstanceName(void* anObject) override
    { 
      T* serializable = static_cast<T*>(anObject);
      return serializable->getName();
    }
    
    virtual void serialize(Serializer* aSerializer, void* anObject) override
    { 
      T* serializable = static_cast<T*>(anObject);
      serializable->serialize(aSerializer);
    }

    virtual void destroy() override { }
  };
//---------------------------------------------------------------------------//
  
  
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
    ObjectName,
    Array,
    Vector,
    Map,
    Vector3,
    Vector4,
    Quaternion,
    Matrix3x3,
    Matrix4x4,
    Serializable,
    SerializablePtr
  };
//---------------------------------------------------------------------------//
  struct DataType
  {
    explicit DataType(EBaseDataType aBaseType, void* aUserData = nullptr) 
      : myBaseType(aBaseType), myUserData(aUserData) {}

    EBaseDataType myBaseType;
    void* myUserData;  // for Serializable types, this pointer hods a metatable-pointer which handles construction/destruction and serialization
  };
//---------------------------------------------------------------------------//
} // end of namespace IO


  template<class T>
  struct Get_DataType
  {
    static IO::DataType get()
    {
      return std::remove_pointer<T>::type::getDataType();
    }
  };
//---------------------------------------------------------------------------//
#define DECLARE_DATATYPE(T, ET) \
  template<> \
  struct Get_DataType<T> \
  { \
    static IO::DataType get() \
    { \
      return IO::DataType(IO::EBaseDataType::ET); \
    } \
  };

  // Base types (without meta-table)
  DECLARE_DATATYPE(int, Int);
  DECLARE_DATATYPE(uint, Uint);
  DECLARE_DATATYPE(float, Float);
  DECLARE_DATATYPE(char, Char);
  DECLARE_DATATYPE(const char*, CString);
  DECLARE_DATATYPE(std::string, String);
  DECLARE_DATATYPE(ObjectName, ObjectName);
  DECLARE_DATATYPE(glm::mat4, Matrix4x4);
  DECLARE_DATATYPE(glm::mat3, Matrix3x3);
  DECLARE_DATATYPE(glm::vec3, Vector3);
  DECLARE_DATATYPE(glm::vec4, Vector4);
  DECLARE_DATATYPE(glm::quat, Quaternion);
#undef DECLARE_DATATYPE
  //---------------------------------------------------------------------------//
  /*
  #define DECLARE_DATATYPE_SERIALIZABLE(T, ET) \
  template<> \
  struct Get_DataType<T> \
  { \
  static IO::DataType get() \
  { \
  return IO::DataType(IO::EBaseDataType::ET, &IO::MetaTableImpl<T>::ourMetaTable); \
  } \
  };

  #define SERIALIZABLE(classT) \
  DECLARE_DATATYPE_SERIALIZABLE(classT, Serializable) \
  DECLARE_DATATYPE_SERIALIZABLE(classT*, Serializable) */

#define SERIALIZABLE(classT) \
  static IO::MetaTableImpl<classT> ourMetaTable; \
  static IO::DataType getDataType();

#define SERIALIZABLE_IMPL(classT) \
  IO::MetaTableImpl<classT> classT::ourMetaTable; \
  IO::DataType classT::getDataType() { return IO::DataType(IO::EBaseDataType::Serializable, &ourMetaTable); }

//---------------------------------------------------------------------------//
}  // end of namespace Fancy

