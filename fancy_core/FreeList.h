#pragma once

#include "FancyCorePrerequisites.h"
#include "Callback.h"
#include <list>

namespace Fancy
{
//---------------------------------------------------------------------------//
  class FreeList
  {
  public:
    struct Page
    {
      uint64 myVirtualOffset;
      uint64 mySize;
    };

    struct FreeElement
    {
      uint64 myVirtualOffset;
      uint64 mySize;
    };

    FreeList(uint aPageSize);
    FreeElement Allocate(uint64 aSize);
    void Free(const FreeElement& anElement);
    
    Callback<void(const Page&)> myOnPageCreated;
    Callback<void(const Page&)> myOnPageRemoved;

  private:
    std::list<FreeElement> myFreeList;
    std::vector<Page> myPages;
    uint myPageSize;
  };
//---------------------------------------------------------------------------//
}

