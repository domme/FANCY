#pragma once

/// A simple Any-impementation that can store any type as a typesafe alternative to void* pointers
// TODO: Specialization for certain types (e.g. const char*)
// TODO: Throw a compile-error for non-POD types that can't be properly stored in the DataStorage (e.g. "deep" types with internal pointers?). Maybe add specializations for the most common of such types like std::vector?

namespace Fancy 
{
//---------------------------------------------------------------------------//
  namespace AnyInternal
  {
    struct VTable
    {
      static VTable* GetDummy() { static VTable ourDummy; return &ourDummy; }

      virtual void Assign(void** aStorage, void* aValue) { };
      virtual bool Equals(void* aValue, void* anOtherValue) const{ return false; }
      virtual void Delete(void** aStorage) { };
    };

    template<class T>
    struct VTable_Buffer final : VTable
    {
      void Assign(void** aStorage, void* aValue) override
      {
        uint8* buf = (uint8*)*aStorage;
        memcpy(buf, aValue, sizeof(T));
      }

      bool Equals(void* aValue, void* anOtherValue) const override
      {
        return memcmp(aValue, anOtherValue, sizeof(T));
      }

      void Delete(void** aStorage) override
      {
        uint8* buf = (uint8*)*aStorage;
        memset(buf, 0u, sizeof(T));
      }
    };

    template <class T>
    struct VTable_Ptr final : VTable
    {
      void Assign(void** aStorage, void* aValue) override
      {
        T** objPtr = (T**)aStorage;
        if (*objPtr == nullptr)
          *objPtr = new T();

        **objPtr = *((T*)aValue);
      }

      bool Equals(void* aValue, void* anOtherValue) const
      {
        return memcmp(aValue, anOtherValue, sizeof(T));
      }

      void Delete(void** aStorage) override
      {
        if (*aStorage == nullptr)
          return;

        T** objPtr = (T**)aStorage;
        delete *objPtr;
        *objPtr = nullptr;
      }
    };
    
    template<class T>
    struct Get_VTable
    {
      static VTable_Ptr<T> ourVTablePtr;
      static VTable_Buffer<T> ourVTableBuffer;
      static VTable* Get(uint aMaxBufferSize)
      {
        if (sizeof(T) <= aMaxBufferSize)
          return &ourVTableBuffer;
        else
          return &ourVTablePtr;
      }
    };
  }
//---------------------------------------------------------------------------//
  template<uint MaxBufferSize>
  class AnySized
  {
    union DataStorage {
      void* myPtr;
      uint8 myBuffer[MaxBufferSize];
    };

    struct VTable
    {
      void(*Delete)(DataStorage*);
      void(*Clone)(DataStorage*, const DataStorage*);
      bool(*IsEqual)(const DataStorage*, const DataStorage*);
    };

    template<class T, bool UsesBuffer>
    struct TypePolicy
    {
      static const T* Cast(const DataStorage& aStorage) { return reinterpret_cast<const T*>(aStorage.myBuffer); }
      static T* Cast(DataStorage& aStorage) { return reinterpret_cast<T*>(aStorage.myBuffer); }

      static void Delete(DataStorage* aStorage) { memset(aStorage->myBuffer, 0u, sizeof(T)); }
      static void Clone(DataStorage* aDstStorage, const DataStorage* aSrcStorage) { memcpy(aDstStorage->myBuffer, aSrcStorage->myBuffer, sizeof(T)); }
      static bool IsEqual(const DataStorage* aStorageLeft, const DataStorage* aStorageRight) { return *Cast(aStorageLeft) == *Cast(aStorageRight); }
    };

    template<class T>
    struct TypePolicy<T, false>
    {
      static const T* Cast(const DataStorage& aStorage) { return reinterpret_cast<const T*>(aStorage.myPtr); }
      static T* Cast(DataStorage& aStorage) { return reinterpret_cast<T*>(aStorage.myPtr); }

      static void Delete(DataStorage* aStorage) { memset(aStorage->myBuffer, 0u, sizeof(T)); }
      static void Clone(DataStorage* aDstStorage, const DataStorage* aSrcStorage) { memcpy(aDstStorage->myBuffer, aSrcStorage->myBuffer, sizeof(T)); }
      static bool IsEqual(const DataStorage* aStorageLeft, const DataStorage* aStorageRight) { return *Cast(aStorageLeft) == *Cast(aStorageRight); }
    };



    

  public:
    AnySized()
      : myVTable(AnyInternal::VTable::GetDummy())
      , myDataStorage{ nullptr } {}

    template<class T>
    void operator=(const T& anObject)
    {
      Init<T>(anObject);
    }

    template<class T>
    bool operator==(const T& anObject) const
    {
      if (HasType<T>())
        return To<T>() == anObject;

      return false;
    }

    template<uint OtherBufferSize>
    bool operator==(const AnySized<OtherBufferSize>& anOtherAny) const
    {
      if (!myVTable != anOtherAny.myVTable)
        return false;

      return myVTable->Equals(myDataStorage.myPtr, anOtherAny.myDataStorage.myPtr);
    }

    template<uint OtherBufferSize>
    void operator=(const AnySized<OtherBufferSize>& anOtherAny)
    {
      if (this == &anOtherAny)
        return;

      myVTable->Delete(&myDataStorage);

      myVTable = anOtherAny.myVTable;
      


    }

    template<class T>
    bool HasType() const
    {
      AnyInternal::VTable* vTableForType = AnyInternal::Get_VTable<T>::Get(MaxBufferSize);
      return myVTable == vTableForType;
    }

    template<class T>
    const T& To()
    {
      ASSERT(HasType<T>(), "Invalid any-cast");
      return *((T*)myDataStorage.myPtr);  
    }

  private:
    template<class T>
    bool HasBufferStorage() const
    {
      return myVTable == &AnyInternal::Get_VTable<T>::ourVTableBuffer;
    }

    template <class T>
    void Init(const T& anObject)
    {
      AnyInternal::VTable* newVTable = AnyInternal::Get_VTable<T>::Get(MaxBufferSize);
      if (newVTable != myVTable)
      {
        myVTable->Delete(&myDataStorage);
        myVTable = AnyInternal::Get_VTable<T>::Get(MaxBufferSize);
      }

      myVTable->Assign(&myDataStorage, &anObject);
    }

    AnyInternal::VTable* myVTable;
    DataStorage myDataStorage;
  };
//---------------------------------------------------------------------------//
  using Any = AnySized<64>;
//---------------------------------------------------------------------------//
}