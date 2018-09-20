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
  bool FreeList::Allocate(uint64 aSize, uint anAlignment, uint64& anAllocatedSizeOut, uint64& aVirtualOffsetOut, uint64& anOffsetInPageOut)
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

        anAllocatedSizeOut = alignedSize;
        aVirtualOffsetOut = it->myVirtualOffset;
        anOffsetInPageOut = offsetInPage;
          
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
    return Allocate(aSize, anAlignment, anAllocatedSizeOut, aVirtualOffsetOut, anOffsetInPageOut);
  }
//---------------------------------------------------------------------------//
  void FreeList::Free(uint64 aVirtualOffset, uint64 aSize)
  {
    auto blockIt = std::find_if(myPages.begin(), myPages.end(), [aVirtualOffset, aSize](const Page& aPage)
    { return aVirtualOffset <= aPage.myVirtualOffset && aVirtualOffset + aSize <= aPage.myVirtualOffset + aPage.mySize; });

    ASSERT(blockIt != myPages.end());

    bool inserted = false;
    for (auto freeChunk = myFreeList.begin(); !inserted && freeChunk != myFreeList.end(); ++freeChunk)
    {
      // A..AX..X B..B --> A....A B..B
      if (freeChunk->myVirtualOffset + freeChunk->mySize == aVirtualOffset)
      {
        freeChunk->mySize += aSize;

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
      else if (freeChunk->myVirtualOffset == aVirtualOffset + aSize)
      {
        freeChunk->myVirtualOffset = aVirtualOffset;
        freeChunk->mySize += aSize;
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

        if (aVirtualOffset > it->myVirtualOffset + it->mySize && aVirtualOffset + aSize < nextIt->myVirtualOffset)
        {
          myFreeList.insert(it, FreeElement{ aVirtualOffset, aSize });
          inserted = true;
        }
      }
    }

    if (!inserted)
    {
      if (myFreeList.empty() || myFreeList.back().myVirtualOffset + myFreeList.back().mySize < aVirtualOffset)
        myFreeList.push_back(FreeElement{ aVirtualOffset, aSize });
      else
        myFreeList.push_front(FreeElement{ aVirtualOffset, aSize });
    }
    
    // Check if we can completely remove any block beyond the first default one
    for (int i = myPages.size() - 1; i > 0; --i)
    {
      const Page& block = myPages[i];
      auto it = std::find_if(myFreeList.begin(), myFreeList.end(), [&block](const FreeElement& aChunk) {
        return aChunk.myVirtualOffset == block.myVirtualOffset && aChunk.mySize == block.mySize;
      });

      if (it != myFreeList.end())
      {
        myDestroyPageDataCallback(block.myData);
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
    Page page { alignedSize, alignedSize};
    myCreatePageDataCallback(page.myData);
    myFreeList.push_back(FreeElement{ page.myVirtualOffset, page.mySize });
    myPages.push_back(page);
  }
//---------------------------------------------------------------------------//
}
