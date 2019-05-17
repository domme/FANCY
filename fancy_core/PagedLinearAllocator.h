#pragma once

#include "FancyCoreDefines.h"
#include "GrowingList.h"

#include <functional>

namespace Fancy
{
/*
 * A freelist-based linear allocator with custom page-data and callback-functions for reacting on creation/destruction of pages.
 * Useful for situations where custom data needs to be associated with each page (e.g. resource-heaps in DX12)
 */
  template<class T>
  class PagedLinearAllocator
  {
  public:
    struct Page
    {
      uint64 myStart;
      uint64 myEnd;
      uint myOpenAllocs;
      T myData;
    };

    struct Block
    {
      uint64 myStart;
      uint64 myEnd;
    };

    PagedLinearAllocator(uint64 aPageSize, std::function<bool(uint64, T&)> aPageDataCreateFn, std::function<void(T&)> aPageDataDestroyFn);
    const Page* FindPage(std::function<bool(const Page&)> aPredicateFn);
    const Page* Allocate(uint64 aSize, uint anAlignment, uint64& anOffsetInPageOut);
    void Free(const Block& aBlock);
    bool IsEmpty() const { return myPages.empty(); }
    Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage);

  //private:
    bool CreateAndAddPage(uint64 aSize);
    static bool IsBlockInPage(const Block& aBlock, const Page& aPage) { return aBlock.myStart >= aPage.myStart && aBlock.myEnd <= aPage.myEnd; }

    const uint64 myPageSize;
    std::function<bool(uint64, T&)> myPageDataCreateFn;
    std::function<void(T&)> myPageDataDestroyFn;

    using FreeListT = GrowingList<Block, 64>;
    using FreeListIterator = typename FreeListT::Iterator;
    FreeListT myFreeList;

    std::vector<Page> myPages;
  };
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
// Implementation
//---------------------------------------------------------------------------//
  template <class T>
  PagedLinearAllocator<T>::PagedLinearAllocator(uint64 aPageSize, std::function<bool(uint64, T&)> aPageDataCreateFn, std::function<void(T&)> aPageDataDestroyFn)
    : myPageSize(aPageSize) 
    , myPageDataCreateFn(std::move(aPageDataCreateFn))
    , myPageDataDestroyFn(std::move(aPageDataDestroyFn))
  {
  }
//---------------------------------------------------------------------------//
  template <class T>
  const typename PagedLinearAllocator<T>::Page* PagedLinearAllocator<T>::FindPage(std::function<bool(const Page&)> aPredicateFn)
  {
    for (const Page& page : myPages)
      if (aPredicateFn(page))
        return &page;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  template <class T>
  const typename PagedLinearAllocator<T>::Page* PagedLinearAllocator<T>::Allocate(uint64 aSize, uint anAlignment, uint64& anOffsetInPageOut)
  {
    const uint64 sizeWithAlignment = MathUtil::Align(aSize, anAlignment);

    for (auto it = myFreeList.Begin(); it != myFreeList.End(); ++it)
    {
      // Make sure to allocate enough space that the offset can be moved upwards to match the alignment in client code
      const uint64 extraSize = MathUtil::Align(it->myStart, anAlignment) - it->myStart;
      const uint64 sizeWithAlignmentAndPadding = sizeWithAlignment + extraSize;
      const uint64 freeBlockSize = it->myEnd - it->myStart;

      if (freeBlockSize >= sizeWithAlignmentAndPadding)
      {
        uint64 offsetInPage = 0u;
        Page* page = GetPageAndOffset(it->myStart, offsetInPage);
        ASSERT(page != nullptr);
        
        it->myStart += sizeWithAlignmentAndPadding;
        if (freeBlockSize == sizeWithAlignmentAndPadding)
          myFreeList.Remove(it);

        anOffsetInPageOut = offsetInPage;
        ++page->myOpenAllocs;
        return page;
      }
    }

    // No free chunks left to satisfy the allocation request: Add a new Page!
    if (!CreateAndAddPage(MathUtil::Align(sizeWithAlignment, myPageSize)))
      return nullptr;

    return Allocate(aSize, anAlignment, anOffsetInPageOut);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void PagedLinearAllocator<T>::Free(const Block& aBlockToFree)
  {
    auto pageIt = std::find_if(myPages.begin(), myPages.end(), [aBlockToFree](const Page& aPage)
    {
      return aBlockToFree.myStart >= aPage.myStart && aBlockToFree.myEnd <= aPage.myEnd;
    });
    ASSERT(pageIt != myPages.end(), "No page found for block to free (start: %, end: %)", aBlockToFree.myStart, aBlockToFree.myEnd);
    Page& page = *pageIt;

    --page.myOpenAllocs;

    if (page.myOpenAllocs == 0)  // Was this the last allocation from the page -> remove the page completely
    {
      for (FreeListIterator it = myFreeList.Begin(), end = myFreeList.End(); it != end; ++it)
      {
        if (IsBlockInPage(*it, page))
        {
          ASSERT(!freePageBlockDeleted, "There should only be at least one free block remaining if the page reports 0 allocs");
          it = myFreeList.Remove(it);
          freePageBlockDeleted = true;
        }

        // If no free block could be deleted from that page, it must mean that the block to free covers the entire page
        ASSERT(freePageBlockDeleted || (aBlockToFree.myStart == page.myStart && aBlockToFree.myEnd == page.myEnd));

        myPageDataDestroyFn(page.myData);
        myPages.erase(pageIt);
      }
    }
    else
    {
      FreeListIterator blockBefore = myFreeList.ReverseFind([aBlockToFree, page](const Block& aBlock) {
        return aBlock.myEnd <= aBlockToFree.myStart;
      });
      FreeListIterator blockAfter = blockBefore.Next();

      const bool canMergeWithBefore = blockBefore != myFreeList.End() && IsBlockInPage(*blockBefore, page) && blockBefore->myEnd == aBlockToFree.myStart;
      const bool canMergeWithAfter = blockAfter != myFreeList.End() && IsBlockInPage(*blockAfter, page) && blockAfter->myStart == aBlockToFree.myEnd;
      if (canMergeWithBefore && canMergeWithAfter)  // [A][X][B]
      {
        blockBefore->myEnd = blockAfter->myEnd;
        myFreeList.Remove(blockAfter);
      }
      else if (canMergeWithBefore)  // [A][X]
      {
        blockBefore->myEnd = aBlockToFree.myEnd;
      }
      else if (canMergeWithAfter) // [X][B]
      {
        blockAfter->myStart = aBlockToFree.myStart;
      }
      else  // Not touching any free block in the page. Iterate the freelist again to find a suitable position
      {
        FreeListIterator it = myFreeList.ReverseFind([aBlockToFree](const Block& aBlock) {
          return aBlock.myEnd <= aBlockToFree.myStart;
        });

        if (it != myFreeList.End())
          myFreeList.AddAfter(it, aBlockToFree);
        else
          myFreeList.AddAfter(myFreeList.Begin(), aBlockToFree);
      }
    }
  }
//---------------------------------------------------------------------------//
  template <class T>
  bool PagedLinearAllocator<T>::CreateAndAddPage(uint64 aSize)
  {
    const uint64 alignedSize = MathUtil::Align(aSize, myPageSize);
    const uint64 pageStart = myPages.empty() ? 0u : myPages.back().myEnd;
    const uint64 pageEnd = pageStart + alignedSize;
    Page page{pageStart, pageEnd, 0u};
    if (!myPageDataCreateFn(alignedSize, page.myData))
      return false;

    myFreeList.Add(Block{page.myStart, page.myEnd});
    myPages.push_back(page);
    return true;
  }
//---------------------------------------------------------------------------//
  template <class T>
  typename PagedLinearAllocator<T>::Page* PagedLinearAllocator<T>::GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage)
  {
    for (Page& existingPage : myPages)
    {
      if (existingPage.myStart <= aVirtualOffset && existingPage.myStart + existingPage.myEnd > aVirtualOffset)
      {
        anOffsetInPage = aVirtualOffset - existingPage.myStart;
        return &existingPage;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
}
