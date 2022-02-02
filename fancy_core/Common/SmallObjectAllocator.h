#pragma once

#include "FancyCoreDefines.h"
#include "AlignedStorage.h"

// TODO: Test THE SmallObjectAllocator more thoroughly

namespace Fancy {
  //---------------------------------------------------------------------------// 
  template<class T>
  class SmallObjectAllocator
  {
  public:
    SmallObjectAllocator(uint aPageSize)
      : myPageSize(aPageSize)
      , myNextFreeEntry(nullptr)
      , myLastAllocatedPage(nullptr)
    {
      AllocateNewPage();
    }
  //---------------------------------------------------------------------------//
    ~SmallObjectAllocator()
    {
      FreeAll();
    }
  //---------------------------------------------------------------------------//
    T* Allocate()
    {
      if (myNextFreeEntry == nullptr)
        AllocateNewPage();

      Entry* newFreeEntry = myNextFreeEntry->myNext;
      T* allocatedVal = new(myNextFreeEntry->myVal.myBytes)T;
      
      myNextFreeEntry = newFreeEntry;
      
      return allocatedVal;
    }
  //---------------------------------------------------------------------------//
    void FreeAll()
    {
      while (myLastAllocatedPage != nullptr)
      {
        AllocatedPage* pageToFree = myLastAllocatedPage;
        myLastAllocatedPage = myLastAllocatedPage->myNext;
        free(pageToFree);
      }

      myNextFreeEntry = nullptr;
    }
  //---------------------------------------------------------------------------//
    void Free(T* anObject)
    {
      anObject->~T();
      Entry* newEntry = reinterpret_cast<Entry*>(anObject);
      newEntry->myNext = myNextFreeEntry;
      myNextFreeEntry = newEntry;
    }
  //---------------------------------------------------------------------------//
  private:
    struct AllocatedPage
    {
      AllocatedPage* myNext;
    };
  //---------------------------------------------------------------------------//
    union Entry
    {
      AlignedStorage<T> myVal;
      Entry* myNext;
    };
  //---------------------------------------------------------------------------//
    uint myPageSize;
    Entry* myNextFreeEntry;
    AllocatedPage* myLastAllocatedPage;
  //---------------------------------------------------------------------------//
    void AllocateNewPage()
    {
      AllocatedPage* newPage = (AllocatedPage*) malloc(sizeof(AllocatedPage) + (sizeof(Entry) * myPageSize));
      newPage->myNext = myLastAllocatedPage;
      myLastAllocatedPage = newPage;

      Entry* newEntries = (Entry*) ((uint8)newPage + sizeof(AllocatedPage));
      for (uint i = 0u; i < myPageSize - 1u; ++i)
      {
        newEntries[i].myNext = &newEntries[i + 1];
      }
      newEntries[myPageSize - 1u].myNext = nullptr;

      myNextFreeEntry = newEntries;
    }
  //---------------------------------------------------------------------------//
  };
//---------------------------------------------------------------------------//
}  // end of namespace Fancy
