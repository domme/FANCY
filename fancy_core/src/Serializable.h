#pragma once

#include "ObjectName.h"
#include "FixedArray.h"

namespace Fancy { namespace IO {

  class Serializer;

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

//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType
  {
    static IO::DataType get()
    {
      return T::getDataType();
    }
  };
//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType<std::shared_ptr<T>>
  {
    static IO::DataType get()
    {
      return T::getDataTypePtr();
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
  struct MetaTable
  {
    virtual ~MetaTable() {}
    virtual void* create() = 0;
    virtual String getTypeName(void* anObject) { return ""; }
    virtual String getInstanceName(void* anObject) { return ""; }
    virtual void serialize(IO::Serializer* aSerializer, void* anObject) = 0;
    virtual void destroy() = 0;
  };
  //---------------------------------------------------------------------------//
  struct MetaTableArray
  {
    virtual ~MetaTableArray() {}
    virtual void* getElement(void* anObject, uint anIndex) = 0;
    virtual uint getSize(void* anObject) = 0;
    virtual void resize(void* anObject, uint aNewSize) = 0;
    virtual IO::DataType getElementDataType() = 0;
  };

  namespace Internal
  {
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

      virtual void serialize(IO::Serializer* aSerializer, void* anObject) override
      {
        T* serializable = static_cast<T*>(anObject);
        serializable->serialize(aSerializer);
      }

      virtual void destroy() override { }

      static MetaTableImpl<T> ourVTable;
    };
    template<class T>
    MetaTableImpl<T> MetaTableImpl<T>::ourVTable;
    //---------------------------------------------------------------------------//
    template<class T>
    struct MetaTableImplPtr : public MetaTable
    {
      virtual ~MetaTableImplPtr<T>() {}
      virtual void* create() override { return nullptr; }

      virtual String getTypeName(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        return (*serializable)->getTypeName();
      }

      virtual String getInstanceName(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        return (*serializable)->getName();
      }

      virtual void serialize(IO::Serializer* aSerializer, void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        (*serializable)->serialize(aSerializer);
      }

      virtual void destroy() override { }

      static MetaTableImplPtr<T> ourVTable;
    };
    template<class T>
    MetaTableImplPtr<T> MetaTableImplPtr<T>::ourVTable;
    //---------------------------------------------------------------------------//
    // Dummy general template:
    template<class T>
    struct MetaTableArrayImpl : public MetaTableArray
    {
      virtual void* getElement(void* anObject, uint anIndex) override
      {
        ASSERT(false);
        return nullptr;
      }

      virtual uint getSize(void* anObject) override
      {
        ASSERT(false);
        return 0u;
      }

      virtual void resize(void* anObject, uint aNewSize) override
      {
        ASSERT(false);
      }

      virtual IO::DataType getElementDataType() override
      {
        ASSERT(false);
        return IO::DataType(IO::EBaseDataType::None);
      }

      static MetaTableArrayImpl<T> ourVTable;
    };
    template<class T>
    MetaTableArrayImpl<T> MetaTableArrayImpl<T>::ourVTable;
    //---------------------------------------------------------------------------//
    template<class T>
    struct MetaTableArrayImpl<std::vector<T>> : public MetaTableArray
    {
      virtual void* getElement(void* anObject, uint anIndex) override
      {
        std::vector<T>* array = reinterpret_cast<std::vector<T>*>(anObject);
        return &((*array)[anIndex]);
      }

      virtual uint getSize(void* anObject) override
      {
        std::vector<T>* array = reinterpret_cast<std::vector<T>*>(anObject);
        return array->size();
      }

      virtual void resize(void* anObject, uint aNewSize) override
      {
        std::vector<T>* array = reinterpret_cast<std::vector<T>*>(anObject);
        return array->resize(aNewSize);
      }

      virtual IO::DataType getElementDataType() override
      {
        return Get_DataType<T>::get();
      }

      static MetaTableArrayImpl<std::vector<T>> ourVTable;
    };
    template<class T>
    MetaTableArrayImpl<std::vector<T>> MetaTableArrayImpl<std::vector<T>>::ourVTable;
    //---------------------------------------------------------------------------//
    template<class T, uint32 Capacity>
    struct MetaTableArrayImpl<FixedArray<T, Capacity>> : public MetaTableArray
    {
      using FixedArrayT = FixedArray < T, Capacity > ;

      virtual void* getElement(void* anObject, uint anIndex) override
      {
        FixedArrayT* array = reinterpret_cast<FixedArrayT*>(anObject);
        return &((*array)[anIndex]);
      }

      virtual uint getSize(void* anObject) override
      {
        FixedArrayT* array = reinterpret_cast<FixedArrayT*>(anObject);
        return array->size();
      }

      virtual void resize(void* anObject, uint aNewSize) override
      {
        FixedArrayT* array = reinterpret_cast<FixedArrayT*>(anObject);
        return array->resize(aNewSize);
      }

      virtual IO::DataType getElementDataType() override
      {
        return Get_DataType<T>::get();
      }

      static MetaTableArrayImpl<FixedArray<T, Capacity>> ourVTable;
    };

    template<class T, uint32 Capacity>
    MetaTableArrayImpl<FixedArray<T, Capacity>> MetaTableArrayImpl<FixedArray<T, Capacity>>::ourVTable;
    //---------------------------------------------------------------------------//
  }  // end of namespace Internal
  //---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType<std::vector<T>>
  {
    static IO::DataType get()
    {
      return IO::DataType(IO::EBaseDataType::Array, &Internal::MetaTableArrayImpl<std::vector<T>>::ourVTable);
    }
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 Capacity>
  struct Get_DataType<FixedArray<T, Capacity>>
  {
    static IO::DataType get()
    {
      return IO::DataType(IO::EBaseDataType::Array, &Internal::MetaTableArrayImpl<FixedArray<T, Capacity>>::ourVTable);
    }
  };
//---------------------------------------------------------------------------//
#define SERIALIZABLE(T) \
  static IO::DataType getDataType() \
  { \
    return IO::DataType(IO::EBaseDataType::Serializable, &Internal::MetaTableImpl<T>::ourVTable); \
  } \
  static IO::DataType getDataTypePtr() \
  { \
    return IO::DataType(IO::EBaseDataType::SerializablePtr, &Internal::MetaTableImplPtr<T>::ourVTable); \
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy

