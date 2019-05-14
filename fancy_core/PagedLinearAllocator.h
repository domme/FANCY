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
      uint64 myVirtualOffset;
      uint64 mySize;
      T myData;
    };

    struct Block
    {
      uint64 myVirtualOffset;
      uint64 mySize;
    };

    PagedLinearAllocator(uint64 aPageSize, std::function<bool(uint64, T&)> aPageDataCreateFn, std::function<void(T&)> aPageDataDestroyFn);
    const Page* FindPage(std::function<bool(const Page&)> aPredicateFn);
    const Page* Allocate(uint64 aSize, uint anAlignment, uint64& anOffsetInPageOut);
    void Free(const Block& aBlock);
    bool IsEmpty() const { return myPages.empty(); }
    const Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage) const;

  private:
    bool CreateAndAddPage(uint64 aSize);

    const uint64 myPageSize;
    std::function<bool(uint64, T&)> myPageDataCreateFn;
    std::function<void(T&)> myPageDataDestroyFn;

    GrowingList<Block, 64> myFreeList;
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
    uint64 alignedSize = MathUtil::Align(aSize, anAlignment);

    for (auto it = myFreeList.Begin(); it != myFreeList.End(); ++it)
    {
      // Make sure to allocate enough space that the offset can be moved upwards to match the alignment in client code
      const uint64 extraSize = MathUtil::Align(it->myVirtualOffset, anAlignment) - it->myVirtualOffset;
      alignedSize += extraSize;

      if (it->mySize >= alignedSize)
      {
        uint64 offsetInPage = 0u;
        const Page* page = GetPageAndOffset(it->myVirtualOffset, offsetInPage);
        ASSERT(page != nullptr);

        if (it->mySize > alignedSize)
        {
          it->mySize -= alignedSize;
          it->myVirtualOffset += alignedSize;
        }
        else if(it->mySize == alignedSize)
        {
          myFreeList.Remove(it);
        }

        anOffsetInPageOut = offsetInPage;
        return page;
      }
    }

    // No free chunks left to satisfy the allocation request: Add a new Page!
    if (!CreateAndAddPage(MathUtil::Align(aSize, myPageSize)))
      return nullptr;

    return Allocate(aSize, anAlignment, anOffsetInPageOut);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void PagedLinearAllocator<T>::Free(const Block& aBlock)
  {
    const uint64 freeBlockStart = aBlock.myVirtualOffset;
    const uint64 freeBlockSize = aBlock.mySize;
    const uint64 freeBlockEnd = freeBlockStart + freeBlockSize;

    auto pageIt = std::find_if(myPages.begin(), myPages.end(), [freeBlockStart, freeBlockEnd](const Page& aPage)
    {
      return aPage.myVirtualOffset <= freeBlockStart && aPage.myVirtualOffset + aPage.mySize >= freeBlockEnd;
    });
    ASSERT(pageIt != myPages.end(), "No page found for block (start: %, end: %)", freeBlockStart, freeBlockEnd);
    const uint64 pageStart = pageIt->myVirtualOffset;
    const uint64 pageEnd = pageIt->myVirtualOffset + pageIt->mySize;

    // Step 0: Try to merge freed block with an existing chunk
    bool inserted = false;
    for (auto currChunk = myFreeList.Begin(); !inserted && currChunk != myFreeList.End(); ++currChunk)
    {
      if (currChunk->myVirtualOffset < pageStart || (currChunk->myVirtualOffset + currChunk->mySize) > pageEnd)  // This chunk is outside of the page the freed block belongs to. Can't merge with that block
        continue;
      
      if ((currChunk->myVirtualOffset + currChunk->mySize) == freeBlockStart) // New block behind existing block: [A][X] -> [ A ]
      {
        currChunk->mySize += freeBlockSize;

        // Does the new, bigger chunk touch with the following chunk? Then merge those if they belong to the same page
        // [ A ][B] -> [  A  ]
        auto nextChunk = currChunk;
        ++nextChunk;

        if (nextChunk != myFreeList.End())
        {
          if ((nextChunk->myVirtualOffset + nextChunk->mySize) <= pageEnd // Next chunk in page?
           && (currChunk->myVirtualOffset + currChunk->mySize) == nextChunk->myVirtualOffset)  // Next chunk directly follows curr chunk?
          {
            currChunk->mySize += nextChunk->mySize;
            myFreeList.Remove(nextChunk);
          }
        }

        inserted = true;
      }

      // CONTINUE HERE!!
        // A..A X..XB..B --> A..A B....B 
      else if (currChunk->myVirtualOffset == freeBlockStart + freeBlockSize)
      {
        currChunk->myVirtualOffset = freeBlockStart;
        currChunk->mySize += freeBlockSize;
        inserted = true;
      }
    }

    // A..A B..B --> A..A C..C B..B
    if (!inserted)
    {
      auto it = myFreeList.Begin();
      for (; it != myFreeList.End(); ++it)
      {
        auto nextIt = it;
        ++nextIt;

        if (nextIt == myFreeList.End())
          break;

        if (freeBlockStart > it->myVirtualOffset + it->mySize && freeBlockStart + freeBlockSize < nextIt->myVirtualOffset)
        {
          myFreeList.AddBefore(it, aBlock);
          inserted = true;
        }
      }
    }

    if (!inserted)
    {
      if (myFreeList.IsEmpty() || myFreeList.Back().myVirtualOffset + myFreeList.Back().mySize < freeBlockStart)
        myFreeList.Add(aBlock);
      else
        myFreeList.AddBefore(myFreeList.Begin(), aBlock);
    }

    // Check if we can completely remove a page
    for (int i = (int)myPages.size() - 1; i >= 0; --i)
    {
      Page& page = myPages[i];
      auto it = std::find_if(myFreeList.Begin(), myFreeList.End(), [&page](const Block& anOtherBlock)
      {
        // BUG: This only checks for free blocks that exactly match the page-size. Free blocks crossing page-borders will not be detected. (But crossing blocks shouldn't be allowed in the first place)
        return anOtherBlock.myVirtualOffset == page.myVirtualOffset && anOtherBlock.mySize == page.mySize; 
      });

      if (it != myFreeList.End())
      {
        myPageDataDestroyFn(page.myData);
        myFreeList.Remove(it);
        myPages.erase(myPages.begin() + i);
      }
    }
  }
//---------------------------------------------------------------------------//
  template <class T>
  bool PagedLinearAllocator<T>::CreateAndAddPage(uint64 aSize)
  {
    const uint64 alignedSize = MathUtil::Align(aSize, myPageSize);
    const uint64 virtualPageOffset = myPages.empty() ? 0u : myPages.back().myVirtualOffset + myPages.back().mySize;
    Page page{virtualPageOffset, alignedSize};
    if (!myPageDataCreateFn(alignedSize, page.myData))
      return false;

    myFreeList.Add(Block{page.myVirtualOffset, page.mySize});
    myPages.push_back(page);
    return true;
  }
//---------------------------------------------------------------------------//
  template <class T>
  const typename PagedLinearAllocator<T>::Page* PagedLinearAllocator<T>::GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage) const
  {
    for (const Page& existingPage : myPages)
    {
      if (existingPage.myVirtualOffset <= aVirtualOffset && existingPage.myVirtualOffset + existingPage.mySize > aVirtualOffset)
      {
        anOffsetInPage = aVirtualOffset - existingPage.myVirtualOffset;
        return &existingPage;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
}
