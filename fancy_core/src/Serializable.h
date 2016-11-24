#pragma once

#include "ObjectName.h"
#include "FixedArray.h"
#include "StaticManagedObject.h"
#include "ObjectFactory.h"
#include "ResourceDesc.h"



namespace Fancy {
struct ResourceDesc;

namespace Rendering {
  class Texture;
}

namespace IO {

  class Serializer;

//---------------------------------------------------------------------------//
  enum class EBaseDataType
  {
    None,

    Int,
    Uint,
    Uint32,
    Uint8,
    Uint16,
    Float,
    Char,
    Bool,
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
    SerializablePtr,
    ResourcePtr,
    StructOrClass,
  };
//---------------------------------------------------------------------------//
  //struct MetaTableStructOrClass
  //{
  //  virtual void serialize(IO::Serializer* aSerializer, void* anObject) = 0;
  //};
////---------------------------------------------------------------------------//
  //template<class T>
  //struct MetaTableStructOrClassImpl : public MetaTableStructOrClass
  //{
  //  virtual void serialize(IO::Serializer* aSerializer, void* anObject) override
  //  {
  //    ((T*)anObject)->serialize(aSerializer);
  //  }
  //
  //  static MetaTableStructOrClassImpl<T> ourVTable;
  //};+
  //template<class T>
  //MetaTableStructOrClassImpl<T> MetaTableStructOrClassImpl<T>::ourVTable;
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
  template<class T, typename = void>
  struct Get_DataType
  {
    static IO::DataType get()
    {
      return T::template getDataType<void>();
    }
  };
//---------------------------------------------------------------------------//
  // Special case for enum types
  template<class T>
  struct Get_DataType<T, typename std::enable_if<std::is_enum<T>::value>::type>
  {
    static IO::DataType get()
    {
      return IO::DataType(IO::EBaseDataType::Uint32);
    }
  };
//---------------------------------------------------------------------------//
  //template<class T>
  //struct Get_DataType<T, typename T::IsSerializable>
  //{
  //  static IO::DataType get()
  //  {
  //    return T::template getDataType<void>();
  //  }
  //};
//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType<std::shared_ptr<T>>
  {
    static IO::DataType get()
    {
      return T::template getDataTypePtr<void>();
    }
  };
//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType<std::shared_ptr<T>*>
  {
    static IO::DataType get()
    {
      return T::template getDataTypePtr<void>();
    }
  };
//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType<T*>
  {
    static IO::DataType get()
    {
      return T::template getDataTypeRawPtr<void>();
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
  DECLARE_DATATYPE(uint32, Uint32);
  DECLARE_DATATYPE(uint8, Uint8);
  DECLARE_DATATYPE(uint16, Uint16);
  DECLARE_DATATYPE(float, Float);
  DECLARE_DATATYPE(char, Char);
  DECLARE_DATATYPE(const char*, CString);
  DECLARE_DATATYPE(std::string, String);
  DECLARE_DATATYPE(String, String);
  DECLARE_DATATYPE(ObjectName, ObjectName);
  DECLARE_DATATYPE(glm::mat4, Matrix4x4);
  DECLARE_DATATYPE(glm::mat3, Matrix3x3);
  DECLARE_DATATYPE(glm::vec3, Vector3);
  DECLARE_DATATYPE(glm::vec4, Vector4);
  DECLARE_DATATYPE(glm::quat, Quaternion);
  DECLARE_DATATYPE(bool, Bool);
#undef DECLARE_DATATYPE


//---------------------------------------------------------------------------//
  struct MetaTable
  {
    virtual ~MetaTable() {}
    virtual void create(void* anObject, const ObjectName& aTypeName, bool& aWasCreated, uint64 aHash = 0u) = 0;
    virtual String getTypeName(void* anObject) { return ""; }
    virtual uint64 getHash(void* anObject) { return 0u; }
    virtual bool isManaged(void* anObject) { return false; }
    virtual void serialize(IO::Serializer* aSerializer, void* anObject) = 0;
    virtual bool isValid(void* anObject) { return true; }
    virtual void invalidate(void* anObject) {}
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
//---------------------------------------------------------------------------//
  struct MetaTableResource
  {
    virtual ~MetaTableResource() {}
    virtual bool IsValid(void* anObject) = 0;
    virtual String GetTypeName(void* anObject) = 0;
    virtual uint64 GetHash(void* anObject) = 0;
    virtual SharedPtr<ResourceDesc> GetDescription(void* anObject) = 0;
  };
//---------------------------------------------------------------------------//
  namespace Internal
  {
    //---------------------------------------------------------------------------//
    // Default Serializable objects
    //---------------------------------------------------------------------------//
    template<class T>
    struct MetaTableImpl : public MetaTable
    {
      virtual void create(void* anObject, const ObjectName& aTypeName, bool& aWasCreated, 
        uint64 aHash = 0u) override { }

      virtual String getTypeName(void* anObject) override
      {
        T* serializable = static_cast<T*>(anObject);
        return serializable->getTypeName();
      }

      virtual bool isManaged(void* anObject) override
      {
        return std::is_base_of<BaseManagedObject, T>::value;
      }

      virtual uint64 getHash(void* anObject) override
      {
        T* serializable = static_cast<T*>(anObject);
        return serializable->GetHash();
      }

      virtual void serialize(IO::Serializer* aSerializer, void* anObject) override
      {
        T* serializable = static_cast<T*>(anObject);
        serializable->serialize(aSerializer);
      }

      static MetaTableImpl<T> ourVTable;
    };
    template<class T>
    MetaTableImpl<T> MetaTableImpl<T>::ourVTable;
//---------------------------------------------------------------------------//
    // Raw ptrs to Serializable objects
    template<class T>
    struct MetaTableImpl<T*> : public MetaTable
    {
      virtual void create(void* anObject, const ObjectName& aTypeName, bool& aWasCreated,
        uint64 aHash = 0u) override 
      {
        T** serializable = static_cast<T**>(anObject);
        (*serializable) = static_cast<T*>(IO::ObjectFactory::create(aTypeName, aWasCreated, aHash));
      }

      virtual bool isValid(void* anObject) override
      {
        T** serializable = static_cast<T**>(anObject);
        return (*serializable) != nullptr;
      }

      virtual void invalidate(void* anObject) override
      {
        T** serializable = static_cast<T**>(anObject);
        (*serializable) = nullptr;
      }

      virtual String getTypeName(void* anObject) override
      {
        T** serializable = static_cast<T**>(anObject);
        return (*serializable)->getTypeName();
      }

      virtual bool isManaged(void* anObject) override
      {
        return std::is_base_of<BaseManagedObject, T>::value;
      }

      virtual uint64 getHash(void* anObject) override
      {
        T** serializable = static_cast<T**>(anObject);
        return (*serializable)->GetHash();
      }

      virtual void serialize(IO::Serializer* aSerializer, void* anObject) override
      {
        T** serializable = static_cast<T**>(anObject);
        (*serializable)->serialize(aSerializer);
      }

      static MetaTableImpl<T*> ourVTable;
    };
    template<class T>
    MetaTableImpl<T*> MetaTableImpl<T*>::ourVTable;
//---------------------------------------------------------------------------//
    // Shared ptrs to Serializable objects
    template<class T>
    struct MetaTableImpl<std::shared_ptr<T>> : public MetaTable
    {
      virtual ~MetaTableImpl<std::shared_ptr<T>>() {}
      
      virtual void create(void* anObject, const ObjectName& aTypeName, bool& aWasCreated,
        uint64 aHash = 0u) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        (*serializable) = std::shared_ptr<T>(static_cast<T*>(IO::ObjectFactory::create(aTypeName, aWasCreated, aHash)));
      }

      virtual bool isValid(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        return (*serializable) != nullptr;
      }

      virtual void invalidate(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        (*serializable) = nullptr;
      }

      virtual bool isManaged(void* anObject) override
      {
        return std::is_base_of<BaseManagedObject, T>::value;
      }

      virtual String getTypeName(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        return (*serializable)->getTypeName();
      }

      virtual uint64 getHash(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        return (*serializable)->GetHash();
      }

      virtual void serialize(IO::Serializer* aSerializer, void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        (*serializable)->serialize(aSerializer);
      }

      static MetaTableImpl<std::shared_ptr<T>> ourVTable;
    };
    template<class T>
    MetaTableImpl<std::shared_ptr<T>> MetaTableImpl<std::shared_ptr<T>>::ourVTable;
  //---------------------------------------------------------------------------//
    // Default resource objects (dummy - should never be instanciated)
    template<class T>
    struct MetaTableResourceImpl : public MetaTableResource
    {
    
    };
//---------------------------------------------------------------------------//
     // Shared pointers to resource objects (should be the only specialization that is ever used)
    template<class T>
    struct MetaTableResourceImpl<std::shared_ptr<T>> : public MetaTableResource
    {
      bool IsValid(void* anObject) override
      {
        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        return (*serializable) != nullptr;
      }

      String GetTypeName(void* anObject) override
      { 
        SharedPtr<ResourceDesc> desc = GetDescription(anObject);
        return desc->GetTypeName().toString();
      }

      uint64 GetHash(void* anObject) override 
      { 
        SharedPtr<ResourceDesc> desc = GetDescription(anObject);
        return desc->GetHash();
      }

      SharedPtr<ResourceDesc> GetDescription(void* anObject) override 
      { 
        typedef typename T::DescT DescT;

        static_assert(std::is_base_of_v<ResourceDesc, DescT>, "Invalid Resource Desc type");

        std::shared_ptr<T>* serializable = static_cast<std::shared_ptr<T>*>(anObject);
        SharedPtr<DescT> desc(new DescT((*serializable)->GetDescription()));
        return desc;
      }
      static MetaTableResourceImpl<std::shared_ptr<T>> ourVTable;
    };
    template<class T>
    MetaTableResourceImpl<std::shared_ptr<T>> MetaTableResourceImpl<std::shared_ptr<T>>::ourVTable;
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  // Array metatables
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
    // Specialization for std::vector
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
    // Specialization for FixedArray
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
    // Specialization for C-Arrays
    template<class T, uint32 Capacity>
    struct MetaTableArrayImpl<T[Capacity]> : public MetaTableArray
    {
      using BuitinArrayT = T[Capacity];

      virtual void* getElement(void* anObject, uint anIndex) override
      {
        BuitinArrayT* array = reinterpret_cast<BuitinArrayT*>(anObject);
        return &((*array)[anIndex]);
      }

      virtual uint getSize(void* anObject) override
      {
        return Capacity;
      }

      virtual void resize(void* anObject, uint aNewSize) override
      {
        ASSERT(aNewSize == Capacity);
      }

      virtual IO::DataType getElementDataType() override
      {
        return Get_DataType<T>::get();
      }

      static MetaTableArrayImpl<T[Capacity]> ourVTable;
    };
    template<class T, uint32 Capacity>
    MetaTableArrayImpl<T[Capacity]> MetaTableArrayImpl<T[Capacity]>::ourVTable;
//---------------------------------------------------------------------------//
  }  // end of namespace Internal
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  template<class T>
  struct Get_DataType<std::vector<T>>
  {
    static IO::DataType get()
    {
      return IO::DataType(IO::EBaseDataType::Array, &Fancy::Internal::MetaTableArrayImpl<std::vector<T>>::ourVTable);
    }
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 Capacity>
  struct Get_DataType<FixedArray<T, Capacity>>
  {
    static IO::DataType get()
    {
      return IO::DataType(IO::EBaseDataType::Array, &Fancy::Internal::MetaTableArrayImpl<FixedArray<T, Capacity>>::ourVTable);
    }
  };
//---------------------------------------------------------------------------//
  template<class T, uint Capacity>
  struct Get_DataTypeBuiltinArray
  {
    static IO::DataType get()
    {
      return IO::DataType(IO::EBaseDataType::Array, &Fancy::Internal::MetaTableArrayImpl<T[Capacity]>::ourVTable);
    }
  };
  //---------------------------------------------------------------------------//
#define SERIALIZABLE(T) \
  enum { IsSerializable = 1 }; \
  template<class dummy> \
  static IO::DataType getDataType() \
  { \
    return IO::DataType(IO::EBaseDataType::Serializable, &Fancy::Internal::MetaTableImpl<T>::ourVTable); \
  } \
  template<class dummy> \
  static IO::DataType getDataTypePtr() \
  { \
    return IO::DataType(IO::EBaseDataType::SerializablePtr, &Fancy::Internal::MetaTableImpl<std::shared_ptr<T>>::ourVTable); \
  } \
  template<class dummy> \
  static IO::DataType getDataTypeRawPtr() \
  { \
    return IO::DataType(IO::EBaseDataType::SerializablePtr, &Fancy::Internal::MetaTableImpl<T*>::ourVTable); \
  }
//---------------------------------------------------------------------------//
#define SERIALIZABLE_RESOURCE(T) \
  enum { IsSerializable = 1 }; \
  using DescT = ##T##Desc; \
  template<class dummy> \
  static IO::DataType getDataTypePtr() \
  { \
    return IO::DataType(IO::EBaseDataType::ResourcePtr, &Fancy::Internal::MetaTableResourceImpl<std::shared_ptr<T>>::ourVTable); \
  }
//---------------------------------------------------------------------------//
}  // end of namespace Fancy

