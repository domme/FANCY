#include "stdafx.h"
#include "DX12Prerequisites.h"
#include "GpuMemoryAllocatorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "MathUtil.h"
#include "AdapterDX12.h"

namespace Fancy
{
  
//---------------------------------------------------------------------------//
  GpuMemoryAllocatorDX12::GpuMemoryAllocatorDX12(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aMemBlockSize)
    : myType(aType)
    , myAccessType(anAccessType)
    , myFreeList(aMemBlockSize, Bind(this, &GpuMemoryAllocatorDX12::OnCreatePageData), Bind(this, &GpuMemoryAllocatorDX12::OnDestroyPageData))
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocatorDX12::~GpuMemoryAllocatorDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 GpuMemoryAllocatorDX12::Allocate(const uint64 aSize, const uint anAlignment)
  {
    GpuMemoryAllocationDX12 allocResult;

    uint64 virtualOffset, allocatedSize, offsetInPage;
    myFreeList.Allocate(aSize, anAlignment, allocatedSize, virtualOffset, offsetInPage);



    
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::Free(GpuMemoryAllocationDX12& anAllocation)
  {
    auto blockIt = std::find_if(myPages.begin(), myPages.end(), [&anAllocation](const Page& anOther)
    { return anOther.myHeap.Get() == anAllocation.myHeap; });

    ASSERT(blockIt != myPages.end());

    const uint64 virtualOffset = blockIt->myVirtualOffset + anAllocation.myOffsetInHeap;

    bool inserted = false;
    // Can the memory blocked be merged with an existing free block?
    for (auto freeChunk = myFreeList.begin(); !inserted && freeChunk != myFreeList.end(); ++freeChunk)
    {
      // Freed memory block directly starts after an exsting free chunk
      if (freeChunk->myVirtualOffset + freeChunk->mySize == virtualOffset)
      {
        freeChunk->mySize += anAllocation.mySize;

        auto nextChunk = freeChunk; 
        ++nextChunk;

        // Freeing the memory removes the "gap" between this free chunk and the next. Remove the next chunk and merge it to its predecessor
        if (nextChunk != myFreeList.end() && nextChunk->myVirtualOffset == freeChunk->myVirtualOffset + freeChunk->mySize)
        {
          freeChunk->mySize += nextChunk->mySize;
          myFreeList.erase(nextChunk);
        }

        inserted = true;
      }

      // Freed memory block ends on an exsiting free chunk (but does not touch the chunk before that)
      else if (freeChunk->myVirtualOffset == virtualOffset + anAllocation.mySize)
      {
        freeChunk->myVirtualOffset = virtualOffset;
        freeChunk->mySize += anAllocation.mySize;
        inserted = true;
      }
    }

    if (!inserted)
    {
      myFreeList.push_back(FreeElement{ virtualOffset, anAllocation.mySize });
      myFreeList.sort([](const FreeElement& a, const FreeElement& b) { return a.myVirtualOffset < b.myVirtualOffset; });
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
        myFreeList.erase(it);
        myPages.erase(myPages.begin() + i);
      }
    }

    anAllocation = {};
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::OnCreatePageData(Microsoft::WRL::ComPtr<ID3D12Heap>& aHeap, uint64 aSize)
  {
    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    const uint64 alignedSize  = MathUtil::Align(aSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

    D3D12_HEAP_DESC heapDesc{ 0u };
    heapDesc.SizeInBytes = alignedSize;
    heapDesc.Flags = Adapter::ResolveHeapFlags(aMemoryType);
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Properties.Type = RenderCore_PlatformDX12::ResolveHeapType(anAccessType);
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    CheckD3Dcall(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&aHeap)));
  }

  void GpuMemoryAllocatorDX12::OnDestroyPageData(Microsoft::WRL::ComPtr<ID3D12Heap>& aHeap)
  {
  }

  //---------------------------------------------------------------------------//
  const GpuMemoryAllocatorDX12::Page* GpuMemoryAllocatorDX12::GetPageAndOffset(uint64 aVirtualOffset, uint64& aOffsetInBlock)
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
}

