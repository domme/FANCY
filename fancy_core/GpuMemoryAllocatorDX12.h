#pragma once

#include "RendererPrerequisites.h"
#include "RenderEnums.h"
#include "PagedLinearAllocator.h"
#include "GpuMemoryAllocationDX12.h"
#include <list>

struct ID3D12Heap;

namespace Fancy
{
//---------------------------------------------------------------------------// 
  class GpuMemoryAllocatorDX12
  {
  public:
    GpuMemoryAllocatorDX12(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aMemBlockSize);
    ~GpuMemoryAllocatorDX12();

    GpuMemoryAllocationDX12 Allocate(const uint64 aSize, const uint anAlignment, const char* aDebugName = nullptr);
    void Free(GpuMemoryAllocationDX12& anAllocation);

  private:
    GpuMemoryType myType;
    CpuMemoryAccessType myAccess;
    PagedLinearAllocator<Microsoft::WRL::ComPtr<ID3D12Heap>> myAllocator;

  #if FANCY_DX12_DEBUG_ALLOCS
    struct AllocDebugInfo
    {
      String myName;
      uint64 myStart;
      uint64 myEnd;
    };
    std::list<AllocDebugInfo> myAllocDebugInfos;
#endif
  };
//---------------------------------------------------------------------------//
}
