#pragma once

#include "FancyCorePrerequisites.h"
#include "Callback.h"
#include <list>
#include "Slot.h"

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

    FreeList(uint aPageSize);
    bool Allocate(uint64 aSize, uint anAlignment, uint64& anAllocatedSizeOut, uint64& aVirtualOffsetOut, uint64& anOffsetInPageOut);
    void Free(uint64 aVirtualOffset, uint64 aSize);
    
    Slot<void(const Page&)> myOnPageCreated;
    Slot<void(const Page&)> myOnPageRemoved;

  private:
    struct FreeElement
    {
      uint64 myVirtualOffset;
      uint64 mySize;
    };

    void CreateAndAddPage(uint64 aSize);
    const Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& aOffsetInPage);

    std::list<FreeElement> myFreeList;
    std::vector<Page> myPages;
    const uint myPageSize;
  };
//---------------------------------------------------------------------------//
}

