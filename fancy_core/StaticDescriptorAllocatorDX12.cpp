#include "fancy_core_precompile.h"
#include "StaticDescriptorAllocatorDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy 
{
//---------------------------------------------------------------------------//
  StaticDescriptorAllocatorDX12::StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptorsPerHeap)
    : PagedLinearAllocator(aNumDescriptorsPerHeap)
    , myHandleIncrementSize(RenderCore::GetPlatformDX12()->GetDevice()->GetDescriptorHandleIncrementSize(aType))
    , myType(aType)
  {
    
  }
//---------------------------------------------------------------------------//
  StaticDescriptorAllocatorDX12::~StaticDescriptorAllocatorDX12()
  {
    ASSERT(IsEmpty(), "There are still static descriptors allocated when destroying the descriptor allocator");
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 StaticDescriptorAllocatorDX12::AllocateDescriptor(const char* aDebugName /* = nullptr*/)
  {
    uint64 descriptorIndexInHeap;
    const Page* page = Allocate(1u, 1u, descriptorIndexInHeap, aDebugName);

    if (page == nullptr)
      return DescriptorDX12{};
    
    const Heap& heapData = eastl::any_cast<const Heap&>(page->myData);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = heapData.myCpuHeapStart.ptr + myHandleIncrementSize * descriptorIndexInHeap;

    DescriptorDX12 descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle.ptr = UINT64_MAX;
    descr.myHeapType = myType;
    descr.myIsManagedByAllocator = true;

    return descr;
  }
//---------------------------------------------------------------------------//
  void StaticDescriptorAllocatorDX12::FreeDescriptor(const DescriptorDX12& aDescriptor)
  {
    ASSERT(aDescriptor.myIsManagedByAllocator && aDescriptor.myHeapType == myType);
    const Page* page = PagedLinearAllocator::FindPage([this, aDescriptor](const Page& aPage) 
    {
      const Heap& heapData = eastl::any_cast<const Heap&>(aPage.myData);
      const uint64 firstHeapAddess = heapData.myCpuHeapStart.ptr;
      const uint64 lastHeapAddress = heapData.myCpuHeapStart.ptr + (aPage.myEnd-aPage.myStart) * myHandleIncrementSize;
      return aDescriptor.myCpuHandle.ptr >= firstHeapAddess && aDescriptor.myCpuHandle.ptr <= lastHeapAddress;
    });
    ASSERT(page != nullptr);
    const Heap& heapData = eastl::any_cast<const Heap&>(page->myData);

    const uint64 addressOffset = aDescriptor.myCpuHandle.ptr - heapData.myCpuHeapStart.ptr;
    ASSERT(addressOffset % myHandleIncrementSize == 0);

    Block block;
    block.myStart = addressOffset / myHandleIncrementSize;
    block.myEnd = block.myStart + 1;
    Free(block);
  }

  bool StaticDescriptorAllocatorDX12::CreatePageData(uint64 aSize, eastl::any& aPageData)
  {
    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    ASSERT(aSize <= UINT_MAX);
    heapDesc.NumDescriptors = static_cast<uint>(aSize);
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0u;
    heapDesc.Type = myType;

    Heap heapData;

    if (!SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapData.myHeap))))
      return false;

    heapData.myCpuHeapStart = heapData.myHeap->GetCPUDescriptorHandleForHeapStart();

    aPageData = heapData;
    return true;
  }
//---------------------------------------------------------------------------//
  void StaticDescriptorAllocatorDX12::DestroyPageData(eastl::any& aPageData)
  {
  }
//---------------------------------------------------------------------------//
}

#endif