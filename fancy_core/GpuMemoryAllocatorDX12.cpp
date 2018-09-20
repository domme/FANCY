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
    , myFreeList(aMemBlockSize)
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocatorDX12::~GpuMemoryAllocatorDX12()
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 GpuMemoryAllocatorDX12::Allocate(const uint64 aSize, const uint anAlignment)
  {
    FreeList::Block block;
    FreeList::Page page;
    myFreeList.Allocate(aSize, anAlignment, block, page);

    GpuMemoryAllocationDX12 allocResult;
    allocResult.myOffsetInHeap = block.myVirtualOffset - page.myVirtualOffset;
    allocResult.mySize = block.mySize;
    
    auto heapIt = myHeaps.find(page.myVirtualOffset);
    if (heapIt == myHeaps.end())
    {
      Microsoft::WRL::ComPtr<ID3D12Heap> heap = CreateHeap(page.mySize);
      allocResult.myHeap = heap.Get();
      myHeaps.insert[page.myVirtualOffset] = heap;
    }
    else
    {
      allocResult.myHeap = heapIt->second.Get();
    }

    return allocResult;
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::Free(GpuMemoryAllocationDX12& anAllocation)
  {
    auto heapIt = std::find_if(myHeaps.begin(), myHeaps.end(), [](auto it))


    FreeList::Page destroyedPage{UINT64_MAX, UINT64_MAX};
    myFreeList.Free()
  }
//---------------------------------------------------------------------------//
  Microsoft::WRL::ComPtr<ID3D12Heap> GpuMemoryAllocatorDX12::CreateHeap(uint64 aSize)
  {
    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
    const uint64 alignedSize  = MathUtil::Align(aSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

    D3D12_HEAP_DESC heapDesc{ 0u };
    heapDesc.SizeInBytes = alignedSize;
    heapDesc.Flags = Adapter::ResolveHeapFlags(myType);
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Properties.Type = RenderCore_PlatformDX12::ResolveHeapType(myAccessType);
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

    Microsoft::WRL::ComPtr<ID3D12Heap> heap;
    CheckD3Dcall(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

    return heap;
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

