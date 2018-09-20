#pragma once

#include "FancyCorePrerequisites.h"
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

    struct Block
    {
      uint64 myVirtualOffset;
      uint64 mySize;
    };

    FreeList(uint aPageSize);
    bool Allocate(uint64 aSize, uint anAlignment, Block& aBlockOut, Page& aPageOut);
    void Free(const Block& aBlock, Page& aDestroyedPageOut);
    
  private:
    void CreateAndAddPage(uint64 aSize);
    const Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& aOffsetInPage);

    std::list<Block> myFreeList;
    std::vector<Page> myPages;
    const uint myPageSize;
  };
//---------------------------------------------------------------------------//
}