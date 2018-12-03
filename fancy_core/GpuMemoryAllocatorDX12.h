#pragma once

#include "RendererPrerequisites.h"
#include "DX12Prerequisites.h"
#include "PagedLinearAllocator.h"
#include <unordered_map>

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
    GpuMemoryAllocatorDX12(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aMemBlockSize);
    ~GpuMemoryAllocatorDX12();

    GpuMemoryAllocationDX12 Allocate(const uint64 aSize, const uint anAlignment, const char* aDebugName = nullptr);
    void Free(GpuMemoryAllocationDX12& anAllocation);

  private:
    PagedLinearAllocator<Microsoft::WRL::ComPtr<ID3D12Heap>> myAllocator;
    GpuMemoryType myType;
    CpuMemoryAccessType myAccess;

  #if FANCY_DX12_DEBUG_ALLOCS
    struct AllocDebugInfo
    {
      String myName;
      uint64 myVirtualOffset;
      uint64 mySize;
    };
    std::list<AllocDebugInfo> myAllocDebugInfos;
#endif
  };
//---------------------------------------------------------------------------//
}
