#pragma once

#include "DX12Prerequisites.h"
#include "GpuResourceDX12.h"

#include <deque>

namespace Fancy { namespace Rendering {
  enum class CommandListType;
} }

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  struct AllocResult
  {
    GpuResourceDX12* myResource;  // The Underlying DX12-Resource
    uint64 myGpuVirtualOffset;    // Offset into the DX12-Resource's GPU-address where this allocation begins
    size_t mySize;                // Size of this allocation
    void* myCpuDataPtr;           // The Cpu-writable address. May be nullptr; Only valid for CPU-writable allocations
  };
//---------------------------------------------------------------------------//
  class GpuDynamicAllocPage : public GpuResourceDX12
  {
  public:
    GpuDynamicAllocPage(ID3D12Resource* aResource, D3D12_RESOURCE_STATES aDefaultUsage, bool aCpuAccessRequired);
    ~GpuDynamicAllocPage();

    void* myCpuDataPtr;   /// Cpu-Writable address to the start of the page
  };
//---------------------------------------------------------------------------//
  enum class GpuDynamicAllocatorType
  {
    GpuOnly = 0,
    CpuWritable = 1,
    Num
  };
//---------------------------------------------------------------------------//
  enum GpuDynamicAllocPageSize
  {
    kCpuPageSize = 2 * 1024 * 1024,  // 2 MB
    kGpuPageSize = 64 * 1024         // 64 KB
  };
//---------------------------------------------------------------------------//
  class GpuDynamicAllocatorDX12
  {
  public:
    GpuDynamicAllocatorDX12(CommandListType aCmdListType, GpuDynamicAllocatorType aType);
    ~GpuDynamicAllocatorDX12();

    AllocResult Allocate(size_t aSizeBytes, size_t anAlignment);

    void CleanupAfterCmdListExecute(uint64 aCmdListDoneFence);

  protected:
    GpuDynamicAllocPage* CreateNewPage() const;
    GpuDynamicAllocatorType myAllocatorType;
    CommandListType myCmdListType;
    uint64 myCurrPageOffsetBytes;
    GpuDynamicAllocPage* myCurrPage;
    std::deque<GpuDynamicAllocPage*> myFullyUsedPages;  /// Pages that have been fully used for allocations
    std::deque<GpuDynamicAllocPage*> myAvailablePages;  /// Pages that were used in one of the previous frames and can be safely re-used now
    std::deque<std::pair<uint64, GpuDynamicAllocPage*>> myWaitingPages;  /// Pages that are still in use by the GPU until their fence-value is passed
  };
//---------------------------------------------------------------------------//
} } }