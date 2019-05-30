#pragma once

#include "RenderEnums.h"
#include "PagedLinearAllocator.h"
#include "GpuMemoryAllocationDX12.h"

struct ID3D12Heap;

namespace Fancy
{
//---------------------------------------------------------------------------// 
  class GpuMemoryAllocatorDX12 : public PagedLinearAllocator
  {
  public:
    GpuMemoryAllocatorDX12(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aMemBlockSize);
    ~GpuMemoryAllocatorDX12();

    GpuMemoryAllocationDX12 Allocate(const uint64 aSize, const uint anAlignment, const char* aDebugName = nullptr);
    void Free(GpuMemoryAllocationDX12& anAllocation);

    bool CreatePageData(uint64 aSize, Any& aPageData) override;
    void DestroyPageData(Any& aPageData) override;

    GpuMemoryType myType;
    CpuMemoryAccessType myAccess;

    

  };
//---------------------------------------------------------------------------//
}
