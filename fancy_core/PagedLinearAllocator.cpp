#include "fancy_core_precompile.h"

#include "PagedLinearAllocator.h"
#include "MathUtil.h"

#include <algorithm>

namespace Fancy
{
  PagedLinearAllocator::PagedLinearAllocator(uint64 aPageSize)
    : myPageSize(aPageSize)
  {
  }
  //---------------------------------------------------------------------------//
  inline PagedLinearAllocator::~PagedLinearAllocator()
  {
  }
  //---------------------------------------------------------------------------//
  const PagedLinearAllocator::Page* PagedLinearAllocator::FindPage(std::function<bool(const Page&)> aPredicateFn)
  {
    for (const Page& page : myPages)
      if (aPredicateFn(page))
        return &page;

    return nullptr;
  }
  //---------------------------------------------------------------------------//
  const typename PagedLinearAllocator::Page* PagedLinearAllocator::Allocate(uint64 aSize, uint anAlignment, uint64& anOffsetInPageOut, const char* aDebugName /*= nullptr*/)
  {
    const uint64 sizeWithAlignment = MathUtil::Align(aSize, anAlignment);

    for (auto it = myFreeList.Begin(); it != myFreeList.Invalid(); ++it)
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
          myFreeList.RemoveGetNext(it);

        anOffsetInPageOut = offsetInPage;
        ++page->myOpenAllocs;

#if CORE_DEBUG_MEMORY_ALLOCATIONS
        AllocDebugInfo debugInfo;
        debugInfo.myName = aDebugName != nullptr ? aDebugName : "Unnamed GPU memory allocation";
        debugInfo.myStart = page->myStart + offsetInPage;
        debugInfo.myEnd = debugInfo.myStart + sizeWithAlignment;
        myAllocDebugInfos.push_back(debugInfo);
#endif
        return page;
      }
    }

    // No free chunks left to satisfy the allocation request: Add a new Page!
    if (!CreateAndAddPage(MathUtil::Align(sizeWithAlignment, myPageSize)))
      return nullptr;

    return Allocate(aSize, anAlignment, anOffsetInPageOut, aDebugName);
  }
  //---------------------------------------------------------------------------//
  void PagedLinearAllocator::Free(const Block& aBlockToFree)
  {
#if CORE_DEBUG_MEMORY_ALLOCATIONS
    auto it = std::find_if(myAllocDebugInfos.begin(), myAllocDebugInfos.end(), [&aBlockToFree](const AllocDebugInfo& anInfo)
    {
      return anInfo.myStart == aBlockToFree.myStart;
    });

    ASSERT(it != myAllocDebugInfos.end());
    myAllocDebugInfos.erase(it);
#endif

    auto pageIt = std::find_if(myPages.begin(), myPages.end(), [aBlockToFree](const Page& aPage)
    {
      return aBlockToFree.myStart >= aPage.myStart && aBlockToFree.myEnd <= aPage.myEnd;
    });
    ASSERT(pageIt != myPages.end(), "No page found for block to free (start: %, end: %)", aBlockToFree.myStart, aBlockToFree.myEnd);
    Page& page = *pageIt;

    --page.myOpenAllocs;

    if (page.myOpenAllocs == 0)  // Was this the last allocation from the page -> remove the page completely
    {
#if CORE_DEBUG_MEMORY_ALLOCATIONS // Validate that the only remaining allocated space is the block to free.
      FreeListIterator firstBlockInPage = myFreeList.Find([&page](const Block& aBlock) { return IsBlockInPage(aBlock, page); });
      FreeListIterator lastBlockInPage;
      if (firstBlockInPage)
      {
        for (FreeListIterator it = firstBlockInPage.Next(); it != myFreeList.Invalid(); ++it)
        {
          if (IsBlockInPage(*it, page))
            lastBlockInPage = it;
        }
      }
      if (!firstBlockInPage)
      {
        // No free blocks in the page, so the block to free must cover the whole page
        ASSERT(page.myStart == aBlockToFree.myStart && page.myEnd == MathUtil::Align(aBlockToFree.myEnd, myPageSize));
      }
      else if (!lastBlockInPage)
      {
        // There is only one free block in this page left. It must be either at the start or at the end of the page and the remaining space must be the block being freed
        const bool isAtStart = firstBlockInPage->myStart == page.myStart;
        const bool isAtEnd = firstBlockInPage->myEnd == page.myEnd;
        ASSERT(isAtStart || isAtEnd);
        ASSERT(!isAtStart || (firstBlockInPage->myEnd == aBlockToFree.myStart) && (page.myEnd == MathUtil::Align(aBlockToFree.myEnd, myPageSize)));
        ASSERT(!isAtEnd || (firstBlockInPage->myStart == aBlockToFree.myEnd) && (page.myStart == aBlockToFree.myStart));
      }
      else  // There are at least two free blocks in the page. There can only be exactly two blocks at the start and end of the page and the space between them must be the block being freed
      {
        ASSERT(firstBlockInPage.Next() == lastBlockInPage); // There must be exactly two free blocks, so the last block must directly follow the first
        ASSERT(firstBlockInPage->myEnd == aBlockToFree.myStart && lastBlockInPage->myStart == aBlockToFree.myEnd);
      }
#endif  // CORE_DEBUG_MEMORY_ALLOCATIONS

      // Remove all free blocks that belong to this page. Remove them in reverse order to make it more likely for myFreeList to re-use existing block-items instead of allocating new ones when adding list-items again
      for (FreeListIterator it = myFreeList.Last(); it != myFreeList.Invalid(); )
      {
        if (IsBlockInPage(*it, page))
          it = myFreeList.RemoveGetPrev(it);
        else
          --it;
      }

      DestroyPageData(page.myData);
      myPages.erase(pageIt);
    }
    else
    {
      FreeListIterator blockBefore = myFreeList.ReverseFind([aBlockToFree](const Block& aBlock) {
        return aBlock.myEnd <= aBlockToFree.myStart;
      });
      FreeListIterator blockAfter = blockBefore ? blockBefore.Next() : myFreeList.Find([aBlockToFree](const Block& aBlock) {
        return aBlock.myStart >= aBlockToFree.myEnd;
      });

      const bool canMergeWithBefore = blockBefore != myFreeList.Invalid() && IsBlockInPage(*blockBefore, page) && blockBefore->myEnd == aBlockToFree.myStart;
      const bool canMergeWithAfter = blockAfter != myFreeList.Invalid() && IsBlockInPage(*blockAfter, page) && blockAfter->myStart == aBlockToFree.myEnd;
      if (canMergeWithBefore && canMergeWithAfter)  // [A][X][B]
      {
        blockBefore->myEnd = blockAfter->myEnd;
        myFreeList.RemoveGetNext(blockAfter);
      }
      else if (canMergeWithBefore)  // [A][X]
      {
        blockBefore->myEnd = aBlockToFree.myEnd;
      }
      else if (canMergeWithAfter) // [X][B]
      {
        blockAfter->myStart = aBlockToFree.myStart;
      }
      else  // Not touching any free block in the page. Insert new block into the freelist
      {
        if (blockBefore)
          myFreeList.AddAfter(blockBefore, aBlockToFree);
        else if (!myFreeList.IsEmpty())
          myFreeList.AddBefore(myFreeList.Begin(), aBlockToFree);
        else
          myFreeList.Add(aBlockToFree);
      }
    }
  }
  //---------------------------------------------------------------------------//
  bool PagedLinearAllocator::CreateAndAddPage(uint64 aSize)
  {
    const uint64 alignedSize = MathUtil::Align(aSize, myPageSize);
    const uint64 pageStart = myPages.empty() ? 0u : myPages.back().myEnd;
    const uint64 pageEnd = pageStart + alignedSize;
    Page page{ pageStart, pageEnd, 0u };
    if (!CreatePageData(alignedSize, page.myData))
      return false;

    myFreeList.Add(Block{ page.myStart, page.myEnd });
    myPages.push_back(page);
    return true;
  }
  //---------------------------------------------------------------------------//
  PagedLinearAllocator::Page* PagedLinearAllocator::GetPageAndOffset(uint64 aVirtualOffset, uint64& anOffsetInPage)
  {
    for (Page& existingPage : myPages)
    {
      if (existingPage.myStart <= aVirtualOffset && existingPage.myEnd > aVirtualOffset)
      {
        anOffsetInPage = aVirtualOffset - existingPage.myStart;
        return &existingPage;
      }
    }

    return nullptr;
  }
  //---------------------------------------------------------------------------//
}