#pragma once

/// A simple Any-impementation that can store any type as a typesafe alternative to void* pointers
// TODO: Specialization for certain types (e.g. const char*)
// TODO: Throw a compile-error for non-POD types that can't be properly stored in the DataStorage (e.g. "deep" types with internal pointers?). Maybe add specializations for the most common of such types like std::vector?

namespace Fancy 
{
  namespace EqualityFallback
  {
    template <class T>
    bool operator==(const T& aLeft, const T& aRight)
    {
      return false;
    }
  }

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
      void(*Clone)(DataStorage*, const DataStorage&);
      bool(*IsEqual)(const DataStorage&, const DataStorage&);
    };

    template<class T>
    static bool IsEqual_Impl(const T& aLeft, const T& aRight)
    {
      using namespace EqualityFallback;
      return aLeft == aRight;
    }

    template<class T, bool UsesBuffer = true>
    struct TypePolicy
    {
      static const T* Cast(const DataStorage& aStorage) { return reinterpret_cast<const T*>(aStorage.myBuffer); }
      static T* Cast(DataStorage& aStorage) { return reinterpret_cast<T*>(aStorage.myBuffer); }

      static void Delete(DataStorage* aStorage) { memset(aStorage->myBuffer, 0u, sizeof(T)); }
      static void Clone(DataStorage* aDstStorage, const DataStorage& aSrcStorage) { memcpy(aDstStorage->myBuffer, aSrcStorage.myBuffer, sizeof(T)); }
      static bool IsEqual(const DataStorage& aStorageLeft, const DataStorage& aStorageRight) { return IsEqual_Impl<T>(*Cast(aStorageLeft), *Cast(aStorageRight)); }
    };

    template<class T>
    struct TypePolicy<T, false>
    {
      static const T* Cast(const DataStorage& aStorage) { return reinterpret_cast<const T*>(aStorage.myPtr); }
      static T* Cast(DataStorage& aStorage) { return reinterpret_cast<T*>(aStorage.myPtr); }

      static void Delete(DataStorage* aStorage) { delete(aStorage->myPtr); aStorage->myPtr = nullptr; }
      static void Clone(DataStorage* aDstStorage, const DataStorage& aSrcStorage) { *Cast(*aDstStorage) = *Cast(aSrcStorage); }
      static bool IsEqual(const DataStorage& aStorageLeft, const DataStorage& aStorageRight) { return IsEqual_Impl<T>(*Cast(aStorageLeft), *Cast(aStorageRight)); }
    };

    template<class T>
    struct Get_Policy
    {
      static TypePolicy<T, sizeof(T) <= MaxBufferSize>& Get()
      {
        static TypePolicy<T, sizeof(T) <= MaxBufferSize> ourPolicy;
        return ourPolicy;
      };
    };

    template<class T>
    struct Get_VTable
    {
      static const VTable* Get()
      {
        static VTable ourVTable
        {
          &Get_Policy<T>::Get().Delete,
          &Get_Policy<T>::Get().Clone,
          &Get_Policy<T>::Get().IsEqual
        };

        return &ourVTable;
      }
    };

  public:
    AnySized()
      : myVTable(nullptr)
      , myDataStorage{ nullptr } {}

    template<class T>
    explicit AnySized(const T& anObject)
    {
      typedef typename std::remove_const_t<std::remove_reference_t<T>> RawType;
      myVTable = Get_VTable<RawType>::Get();

      if (sizeof(RawType) <= MaxBufferSize)
        new(myDataStorage.myBuffer) RawType(anObject);
      else
        myDataStorage.myPtr = new RawType(anObject);
    }

    template<class T>
    void operator=(const T& anObject)
    {
      if (myVTable != nullptr)
        myVTable->Delete(&myDataStorage);

      using RawType = std::remove_const_t<std::remove_reference_t<T>>;
      myVTable = Get_VTable<RawType>::Get();

      if (sizeof(RawType) <= MaxBufferSize)
        new (myDataStorage.myBuffer) RawType(anObject);
      else
        myDataStorage.myPtr = new RawType(anObject);
    }

    template<class T>
    bool HasType() const
    {
      using RawType = std::remove_const_t<std::remove_reference_t<T>>;
      return myVTable == Get_VTable<RawType>::Get();
    }

    template<class T>
    const T& To()
    {
      using RawType = std::remove_const_t<std::remove_reference_t<T>>;
      //      ASSERT(HasType<RawType>());
      if (sizeof(RawType) <= MaxBufferSize)
        return *reinterpret_cast<const RawType*>(myDataStorage.myBuffer);
      else
        return *reinterpret_cast<const RawType*>(myDataStorage.myPtr);
    }

    template<class T>
    bool operator==(const T& anObject) const
    {
      return To<T>() == anObject;
    }

    bool operator==(const AnySized& anOtherAny) const
    {
      if (myVTable == nullptr)
        return anOtherAny.myVTable == nullptr;

      return myVTable->IsEqual(myDataStorage, anOtherAny.myDataStorage);
    }

    void operator=(const AnySized& anOtherAny)
    {
      if (&anOtherAny == this)
        return;

      if (myVTable != nullptr)
        myVTable->Delete(&myDataStorage);

      myVTable = anOtherAny.myVTable;
      myVTable->Clone(&myDataStorage, anOtherAny.myDataStorage);
    }

  private:
    const VTable* myVTable;
    DataStorage myDataStorage;
  };
//---------------------------------------------------------------------------//
  using Any = AnySized<64>;
//---------------------------------------------------------------------------//
}