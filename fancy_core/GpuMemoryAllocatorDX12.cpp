#include "fancy_core_precompile.h"
#include "GpuMemoryAllocatorDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"
#include "AdapterDX12.h"

namespace Fancy
{
//---------------------------------------------------------------------------//  
  GpuMemoryAllocatorDX12::GpuMemoryAllocatorDX12(GpuMemoryType aType, CpuMemoryAccessType anAccessType, uint64 aPageSize)
    : PagedLinearAllocator(aPageSize)
    , myType(aType)
    , myAccess(anAccessType)
  {
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocatorDX12::~GpuMemoryAllocatorDX12()
  {
    ASSERT(PagedLinearAllocator::IsEmpty(), "There are still gpu-resources allocated when destroying the memory allocator");
  }
//---------------------------------------------------------------------------//
  GpuMemoryAllocationDX12 GpuMemoryAllocatorDX12::Allocate(const uint64 aSize, const uint anAlignment, const char* aDebugName /*= nullptr*/)
  {
    uint64 offsetInPage;
    const Page* page = PagedLinearAllocator::Allocate(aSize, anAlignment, offsetInPage, aDebugName);
    if (page == nullptr)
      return GpuMemoryAllocationDX12{};
    
    GpuMemoryAllocationDX12 allocResult;
    allocResult.myOffsetInHeap = offsetInPage;
    allocResult.mySize = aSize;
    allocResult.myHeap = page->myData.To<Microsoft::WRL::ComPtr<ID3D12Heap>>().Get();

    return allocResult;
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::Free(GpuMemoryAllocationDX12& anAllocation)
  {
    const Page* page = FindPage([&](const Page& aPage) {
      return anAllocation.myHeap == aPage.myData.To<Microsoft::WRL::ComPtr<ID3D12Heap>>().Get();
    });

    ASSERT(page != nullptr);

    Block block;
    block.myStart = page->myStart + anAllocation.myOffsetInHeap;
    block.myEnd = block.myStart + anAllocation.mySize;
    PagedLinearAllocator::Free(block);
  }
//---------------------------------------------------------------------------//
  bool GpuMemoryAllocatorDX12::CreatePageData(uint64 aSize, Any& aPageData)
  {
    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
    const uint64 alignedSize = MathUtil::Align(aSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

    D3D12_HEAP_DESC heapDesc{ 0u };
    heapDesc.SizeInBytes = alignedSize;
    heapDesc.Flags = Adapter::ResolveHeapFlags(myType);
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Properties.Type = RenderCore_PlatformDX12::ResolveHeapType(myAccess);
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

    Microsoft::WRL::ComPtr<ID3D12Heap> heap;
    if (!SUCCEEDED(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap))))
      return false;

    aPageData = heap;
    return true;
  }
//---------------------------------------------------------------------------//
  void GpuMemoryAllocatorDX12::DestroyPageData(Any& /*aPageData*/)
  {
  }
//---------------------------------------------------------------------------//
}