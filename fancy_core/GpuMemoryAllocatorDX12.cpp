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
  using Page = PagedLinearAllocator<Microsoft::WRL::ComPtr<ID3D12Heap>>::Page;
  using Block = PagedLinearAllocator<Microsoft::WRL::ComPtr<ID3D12Heap>>::Block;
 //---------------------------------------------------------------------------//
  bool CreateHeap(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aSize, Microsoft::WRL::ComPtr<ID3D12Heap>& aHeapOut)
  {
    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
    const uint64 alignedSize  = MathUtil::Align(aSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

    D3D12_HEAP_DESC heapDesc{ 0u };
    heapDesc.SizeInBytes = alignedSize;
    heapDesc.Flags = Adapter::ResolveHeapFlags(aType);
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Properties.Type = RenderCore_PlatformDX12::ResolveHeapType(anAccessType);
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

    Microsoft::WRL::ComPtr<ID3D12Heap> heap;
    if (!SUCCEEDED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap))))
      return false;

    aHeapOut = heap;
    return true;
  }
//---------------------------------------------------------------------------//
  void DestroyHeap(Microsoft::WRL::ComPtr<ID3D12Heap>& /*aHeap*/)
  {
    // No special treatment needed here. Resource will be released by the smartptr
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocatorDX12::GpuMemoryAllocatorDX12(GpuMemoryType aType, GpuMemoryAccessType anAccessType, uint64 aMemBlockSize)
    : myType(aType)
    , myAccessType(anAccessType)
    , myAllocator(aMemBlockSize, 
      [aType, anAccessType](uint64 aSize, Microsoft::WRL::ComPtr<ID3D12Heap>& aHeapOut){return CreateHeap(aType, anAccessType, aSize, aHeapOut); }
      , DestroyHeap)
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 GpuMemoryAllocatorDX12::Allocate(const uint64 aSize, const uint anAlignment)
  {
    uint64 offsetInPage;
    const Page* page = myAllocator.Allocate(aSize, anAlignment, offsetInPage);
    if (page == nullptr)
      return GpuMemoryAllocationDX12{};
    
    GpuMemoryAllocationDX12 allocResult;
    allocResult.myOffsetInHeap = offsetInPage;
    allocResult.mySize = aSize;
    allocResult.myHeap = page->myData.Get();
    return allocResult;
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::Free(GpuMemoryAllocationDX12& anAllocation)
  {
    const Page* page = myAllocator.FindPage([&](const Page& aPage) {
      return anAllocation.myHeap == aPage.myData.Get();
    });

    ASSERT(page != nullptr);

    Block block;
    block.myVirtualOffset = page->myVirtualOffset + anAllocation.myOffsetInHeap;
    block.mySize = anAllocation.mySize;
    myAllocator.Free(block);
  }
//---------------------------------------------------------------------------//
}

