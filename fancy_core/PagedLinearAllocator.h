#pragma once

#include "FancyCoreDefines.h"
#include "GrowingList.h"
#include "Any.h"

#include "EASTL/vector.h"
#include "EASTL/list.h"
#include "EASTL/functional.h"

namespace Fancy
{
/*
 * A freelist-based linear allocator with custom page-data and callback-functions for reacting on creation/destruction of pages.
 * Useful for situations where custom data needs to be associated with each page (e.g. resource-heaps in DX12)
 */
  class PagedLinearAllocator
  {
  public:
    struct Page
    {
      uint64 myStart;
      uint64 myEnd;
      uint myOpenAllocs;
      Any myData;
    };

    struct Block
    {
      uint64 myStart;
      uint64 myEnd;
    };

    PagedLinearAllocator(uint64 aPageSize);
    virtual ~PagedLinearAllocator();

    const Page* FindPage(eastl::function<bool(const Page&)> aPredicateFn);
    const Page* Allocate(uint64 aSize, uint anAlignment, uint64& anOffsetInPageOut, const char* aDebugName = nullptr);
    void Free(const Block& aBlock);
    bool IsEmpty() const { return myPages.empty(); }
    Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage);

  //private:

    virtual bool CreatePageData(uint64 aSize, Any& aPageData) = 0;
    virtual void DestroyPageData(Any& aPageData) = 0;

    bool CreateAndAddPage(uint64 aSize);
    static bool IsBlockInPage(const Block& aBlock, const Page& aPage) { return aBlock.myStart >= aPage.myStart && aBlock.myEnd <= aPage.myEnd; }

    const uint64 myPageSize;
    using FreeListT = GrowingList<Block, 64>;
    using FreeListIterator = typename FreeListT::Iterator;
    FreeListT myFreeList;

#if CORE_DEBUG_MEMORY_ALLOCATIONS
    struct AllocDebugInfo
    {
      String myName;
      uint64 myStart;
      uint64 myEnd;
    };
    eastl::list<AllocDebugInfo> myAllocDebugInfos;
#endif  // CORE_DEBUG_MEMORY_ALLOCATIONS

    eastl::vector<Page> myPages;
  };
//---------------------------------------------------------------------------//
}
