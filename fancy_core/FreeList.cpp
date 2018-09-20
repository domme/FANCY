#include "stdafx.h"
#include "FreeList.h"
#include "MathUtil.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  FreeList::FreeList(uint aPageSize)
    : myPageSize(aPageSize)
  {
  }
//---------------------------------------------------------------------------//
  bool FreeList::Allocate(uint64 aSize, uint anAlignment, Block& aBlockOut, Page& aPageOut)
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

        aBlockOut.myVirtualOffset = it->myVirtualOffset;
        aBlockOut.mySize = alignedSize;
        aPageOut = *page;
          
        if (it->mySize > alignedSize)
        {
          it->mySize -= alignedSize;
          it->myVirtualOffset += alignedSize;
        }
        else
        {
          myFreeList.erase(it);
        }

        return true;
      }
    }

    // No free chunks left to statisfy the allocation request: Add a new Page!
    CreateAndAddPage(MathUtil::Align(aSize, myPageSize));
    return Allocate(aSize, anAlignment, aBlockOut, aPageOut);
  }
//---------------------------------------------------------------------------//
  void FreeList::Free(const Block& aBlock, Page& aDestroyedPageOut)
  {
    const uint64 virtualOffset = aBlock.myVirtualOffset;
    const uint64 size = aBlock.mySize;

    auto blockIt = std::find_if(myPages.begin(), myPages.end(), [virtualOffset, size](const Page& aPage)
    { return virtualOffset <= aPage.myVirtualOffset && virtualOffset + size <= aPage.myVirtualOffset + aPage.mySize; });

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
        if (nextChunk != myFreeList.end() && nextChunk->myVirtualOffset == freeChunk->myVirtualOffset + freeChunk->mySize)
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
    
    // Check if we can completely remove any block beyond the first default one
    bool hasRemovedPage = false;
    for (int i = myPages.size() - 1; i > 0; --i)
    {
      const Page& page = myPages[i];
      auto it = std::find_if(myFreeList.begin(), myFreeList.end(), [&page](const Block& anOtherBlock) {
        return anOtherBlock.myVirtualOffset == page.myVirtualOffset && anOtherBlock.mySize == page.mySize;
      });

      if (it != myFreeList.end())
      {
        ASSERT(!hasRemovedPage); // Something is fishy if we find two pages to remove if we only free one block
        hasRemovedPage = true;

        aDestroyedPageOut = *(myPages.begin() + i);
        myFreeList.erase(it);
        myPages.erase(myPages.begin() + i);
      }
    }
  }
//---------------------------------------------------------------------------//
  const FreeList::Page* FreeList::GetPageAndOffset(uint64 aVirtualOffset, uint64& aOffsetInBlock)
  {
    for (const Page& existingPage : myPages)
    {
      if (existingPage.myVirtualOffset <= aVirtualOffset && existingPage.myVirtualOffset + existingPage.mySize > aVirtualOffset)
      {
        aOffsetInBlock = aVirtualOffset - existingPage.myVirtualOffset;
        return &existingPage;
      }
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  void FreeList::CreateAndAddPage(uint64 aSize)
  {
    const uint64 alignedSize  = MathUtil::Align(aSize, myPageSize);
    const uint64 virtualPageOffset = myPages.empty() ? 0u : myPages.back().myVirtualOffset + myPages.back().mySize; 
    const Page page{ virtualPageOffset, alignedSize};
    myFreeList.push_back(Block{ page.myVirtualOffset, page.mySize });
    myPages.push_back(page);
  }
//---------------------------------------------------------------------------//
}
