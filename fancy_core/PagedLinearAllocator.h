#pragma once

#include "FancyCoreDefines.h"

#include <list>
#include <functional>

namespace Fancy
{
//---------------------------------------------------------------------------//
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
    bool IsEmpty() const { return myPages.size() == 0u; }
    const Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage) const;

  private:
    bool CreateAndAddPage(uint64 aSize);

    const uint64 myPageSize;
    std::function<bool(uint64, T&)> myPageDataCreateFn;
    std::function<void(T&)> myPageDataDestroyFn;

    std::list<Block> myFreeList;
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

    for (auto it = myFreeList.begin(); it != myFreeList.end(); ++it)
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
        else
        {
          myFreeList.erase(it);
        }

        anOffsetInPageOut = offsetInPage;
        return page;
      }
    }

    // No free chunks left to statisfy the allocation request: Add a new Page!
    if (!CreateAndAddPage(MathUtil::Align(aSize, myPageSize)))
      return nullptr;

    return Allocate(aSize, anAlignment, anOffsetInPageOut);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void PagedLinearAllocator<T>::Free(const Block& aBlock)
  {
    const uint64 virtualOffset = aBlock.myVirtualOffset;
    const uint64 size = aBlock.mySize;

    auto blockIt = std::find_if(myPages.begin(), myPages.end(), [virtualOffset, size](const Page& aPage)
    {
      return aPage.myVirtualOffset <= virtualOffset && aPage.myVirtualOffset + aPage.mySize
        >= virtualOffset + size;
    });

    ASSERT(blockIt != myPages.end());

    bool inserted = false;
    for (auto freeChunk = myFreeList.begin(); !inserted && freeChunk != myFreeList.end(); ++freeChunk)
    {
      // A..AX..X B..B --> A....A B..B
      if (freeChunk->myVirtualOffset + freeChunk->mySize == virtualOffset)
      {
        freeChunk->mySize += size;

        auto nextChunk = freeChunk;
        ++nextChunk;

        // A....AB..B --> A......A
        if (nextChunk != myFreeList.end() && nextChunk->myVirtualOffset == freeChunk->myVirtualOffset + freeChunk->
          mySize)
        {
          freeChunk->mySize += nextChunk->mySize;
          myFreeList.erase(nextChunk);
        }

        inserted = true;
      }
        // A..A X..XB..B --> A..A B....B 
      else if (freeChunk->myVirtualOffset == virtualOffset + size)
      {
        freeChunk->myVirtualOffset = virtualOffset;
        freeChunk->mySize += size;
        inserted = true;
      }
    }

    // A..A B..B --> A..A C..C B..B
    if (!inserted)
    {
      auto it = myFreeList.begin();
      for (; it != myFreeList.end(); ++it)
      {
        auto nextIt = it;
        ++nextIt;

        if (nextIt == myFreeList.end())
          break;

        if (virtualOffset > it->myVirtualOffset + it->mySize && virtualOffset + size < nextIt->myVirtualOffset)
        {
          myFreeList.insert(it, aBlock);
          inserted = true;
        }
      }
    }

    if (!inserted)
    {
      if (myFreeList.empty() || myFreeList.back().myVirtualOffset + myFreeList.back().mySize < virtualOffset)
        myFreeList.push_back(aBlock);
      else
        myFreeList.push_front(aBlock);
    }

    // Check if we can completely remove a page
    for (int i = (int)myPages.size() - 1; i >= 0; --i)
    {
      Page& page = myPages[i];
      auto it = std::find_if(myFreeList.begin(), myFreeList.end(), [&page](const Block& anOtherBlock)
      {
        return anOtherBlock.myVirtualOffset == page.myVirtualOffset && anOtherBlock.mySize == page.mySize;
      });

      if (it != myFreeList.end())
      {
        myPageDataDestroyFn(page.myData);
        myFreeList.erase(it);
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

    myFreeList.push_back(Block{page.myVirtualOffset, page.mySize});
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
