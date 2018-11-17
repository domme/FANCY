#pragma once

#include <utility>

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
      void(*Move)(DataStorage*, DataStorage&);
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

      static void Delete(DataStorage* aStorage) { Cast(*aStorage)->~T(); memset(aStorage->myBuffer, 0u, sizeof(T)); }
      static void Clone(DataStorage* aDstStorage, const DataStorage& aSrcStorage) { new((void*)aDstStorage->myBuffer) T(*Cast(aSrcStorage)); }
      static void Move(DataStorage* aDstStorage, DataStorage& aSrcStorage) { new((void*)aDstStorage->myBuffer) T(std::move(*Cast(aSrcStorage))); }
      static bool IsEqual(const DataStorage& aStorageLeft, const DataStorage& aStorageRight) { return IsEqual_Impl<T>(*Cast(aStorageLeft), *Cast(aStorageRight)); }
    };

    template<class T>
    struct TypePolicy<T, false>
    {
      static const T* Cast(const DataStorage& aStorage) { return reinterpret_cast<const T*>(aStorage.myPtr); }
      static T* Cast(DataStorage& aStorage) { return reinterpret_cast<T*>(aStorage.myPtr); }
      
      static void Delete(DataStorage* aStorage) { delete(Cast(*aStorage)); aStorage->myPtr = nullptr; }
      static void Clone(DataStorage* aDstStorage, const DataStorage& aSrcStorage) { aDstStorage->myPtr = new T(*Cast(aSrcStorage)); }
      static void Move(DataStorage* aDstStorage, DataStorage& aSrcStorage) { aDstStorage->myPtr = aSrcStorage.myPtr; aSrcStorage.myPtr = nullptr; }
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
          &Get_Policy<T>::Get().Move,
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
      using RawType = std::remove_const_t<std::remove_reference_t<T>>;

      myVTable = Get_VTable<RawType>::Get();
      InitDataStorage(anObject);
    }

    template<class T>
    void operator=(const T& anObject)
    {
      if (!IsEmpty())
        myVTable->Delete(&myDataStorage);

      using RawType = std::remove_const_t<std::remove_reference_t<T>>;
      myVTable = Get_VTable<RawType>::Get();
      InitDataStorage(anObject);
    }

    void operator=(const AnySized& anOtherAny)
    {
      if (&anOtherAny == this)
        return;

      if (!IsEmpty())
        myVTable->Delete(&myDataStorage);

      myVTable = anOtherAny.myVTable;
      myVTable->Clone(&myDataStorage, anOtherAny.myDataStorage);
    }

    void operator=(const AnySized&& anOtherAny)
    {
      if (&anOtherAny == this)
        return;

      if (!IsEmpty())
        myVTable->Delete(&myDataStorage);

      myVTable = anOtherAny.myVTable;
      myVTable->Move(&myDataStorage, anOtherAny.myDataStorage);
    }

    bool IsEmpty() const
    {
      return myVTable == nullptr;
    }

    template<class T>
    bool HasType() const
    {
      using RawType = std::remove_const_t<std::remove_reference_t<T>>;
      return myVTable == Get_VTable<RawType>::Get();
    }

    template<class T>
    const T& To() const
    {
      ASSERT(!IsEmpty(), "Any is empty!");

      using RawType = std::remove_const_t<std::remove_reference_t<T>>;

      if (sizeof(RawType) <= MaxBufferSize)
        return *reinterpret_cast<const T*>(myDataStorage.myBuffer);
      else
        return *reinterpret_cast<const T*>(myDataStorage.myPtr);
    }

    template<class T>
    bool operator==(const T& anObject) const
    {
      return !IsEmpty() && To<T>() == anObject;
    }

    bool operator==(const AnySized& anOtherAny) const
    {
      if (IsEmpty())
        return anOtherAny.IsEmpty();

      return myVTable->IsEqual(myDataStorage, anOtherAny.myDataStorage);
    }

  private:
    template<class T>
    void InitDataStorage(const T& anObject)
    {
      using RawType = std::remove_const_t<std::remove_reference_t<T>>;

      if (sizeof(RawType) <= MaxBufferSize)
        new (myDataStorage.myBuffer) RawType(anObject);
      else
        myDataStorage.myPtr = new RawType(anObject);
    }

    const VTable* myVTable;
    DataStorage myDataStorage;
  };
//---------------------------------------------------------------------------//
  using Any = AnySized<64>;
//---------------------------------------------------------------------------//
}