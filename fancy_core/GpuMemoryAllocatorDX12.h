#pragma once

#include "RendererPrerequisites.h"
#include "DX12Prerequisites.h"
#include <list>

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct GpuMemoryAllocationDX12
  {
    GpuMemoryAllocationDX12() : myHeap(nullptr), myOffsetInHeap(0u), mySize(0u) {}
    ID3D12Heap* myHeap;
    uint64 myOffsetInHeap;
    uint64 mySize;
  };
//---------------------------------------------------------------------------// 
  class GpuMemoryAllocatorDX12
  {
  public:
    GpuMemoryAllocatorDX12(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aMemBlockSize);
    ~GpuMemoryAllocatorDX12();

    GpuMemoryAllocationDX12 Allocate(const uint64 aSize, const uint anAlignment);
    void Free(GpuMemoryAllocationDX12& anAllocation);

  private:
    struct Page
    {
      uint64 myVirtualOffset;
      uint64 mySize;
      Microsoft::WRL::ComPtr<ID3D12Heap> myHeap;
    };

    struct FreeElement
    {
      uint64 myVirtualOffset;
      uint64 mySize;
    };

    bool CreateAndAddPage(uint64 aSize);
    const Page* GetPageAndOffset(uint64 aVirtualOffset, uint64& aOffsetInPage);
    
    GpuMemoryType myType;
    GpuMemoryAccessType myAccessType;
    uint64 myDefaultPageSize;
    std::list<FreeElement> myFreeList;
    std::vector<Page> myPages;
  };
//---------------------------------------------------------------------------//
}
