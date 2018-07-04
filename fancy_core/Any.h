#pragma once

namespace Fancy 
{
//---------------------------------------------------------------------------//
  namespace AnyInternal
  {
    struct VTable
    {
      static VTable* GetDummy() { static VTable ourDummy; return &ourDummy; }
      virtual void Delete(void** aStorage) { };
    };

    template<class T>
    struct VTable_Buffer : VTable
    {
      void Delete(void** aStorage) override
      {
        uint8* buf = (uint8*)*aStorage;
        memset(buf, 0u, sizeof(T));
      }
    };

    template <class T>
    struct VTable_Ptr : VTable
    {
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
    struct Get_VTablePtr {
      static VTable_Ptr<T> ourVTable;
    };

    template<class T>
    struct Get_VTableBuffer {
      static VTable_Buffer<T> ourVTable;
    };
  }
//---------------------------------------------------------------------------//
  template<uint MinSize>
  class AnySized
  {
  public:
    AnySized()
      : myVTable(AnyInternal::VTable::GetDummy())
      , myDataStorage{ nullptr } {}

    template<class T>
    void operator=(const T& anObject)
    {
      
    }

  private:
    union DataStorage {
      void* myPtr;
      uint8 myBuffer[MinSize];
    };

    template <class T>
    void Init(const T& anObject)
    {
      myVTable->Delete(&myDataStorage);

      if (sizeof(T) <= MinSize)
      {
        myVTable = &AnyInternal::Get_VTableBuffer<T>::ourVTable;
        memcpy(myDataStorage.myBuffer, &anObject, sizeof(T));
      }
      else
      {
        myVTable = &AnyInternal::Get_VTablePtr<T>::ourVTable;

      }
        

    }

    AnyInternal::VTable* myVTable;
    DataStorage myDataStorage;
  };
//---------------------------------------------------------------------------//
  using Any = AnySized<64>;
//---------------------------------------------------------------------------//
}