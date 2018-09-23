#include "stdafx.h"

#include "StaticDescriptorAllocatorDX12.h"
#include "DX12Prerequisites.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  using Page = PagedLinearAllocator<StaticDescriptorAllocatorDX12::Heap>::Page;
  using Block = PagedLinearAllocator<StaticDescriptorAllocatorDX12::Heap>::Block;
//---------------------------------------------------------------------------//
  bool CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptors, StaticDescriptorAllocatorDX12::Heap& aHeapOut)
  {
    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = aNumDescriptors;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    heapDesc.NodeMask = 0u;
    heapDesc.Type = aType;
    
    if (!SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&aHeapOut.myHeap))))
      return false;

    aHeapOut.myCpuHeapStart = aHeapOut.myHeap->GetCPUDescriptorHandleForHeapStart();
    aHeapOut.myGpuHeapStart = aHeapOut.myHeap->GetGPUDescriptorHandleForHeapStart();

    return true;
  }
//---------------------------------------------------------------------------//
  StaticDescriptorAllocatorDX12::StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptorsPerHeap)
    : myAllocator(
        aNumDescriptorsPerHeap, 
        [aType](uint64 aNumDescriptors, StaticDescriptorAllocatorDX12::Heap& aHeapOut) { return CreateDescriptorHeap(aType, aNumDescriptors, aHeapOut); }, 
        [](StaticDescriptorAllocatorDX12::Heap& aHeapToDestroy) { })
    , myHandleIncrementSize(RenderCore::GetPlatformDX12()->GetDevice()->GetDescriptorHandleIncrementSize(aType))
    , myType(aType)
  {
    
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 StaticDescriptorAllocatorDX12::AllocateDescriptor()
  {
    uint64 descriptorIndexInHeap;
    const Page* page = myAllocator.Allocate(1u, 1u, descriptorIndexInHeap);

    if (page == nullptr)
      return DescriptorDX12{};

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = page->myData.myCpuHeapStart.ptr + myHandleIncrementSize * descriptorIndexInHeap;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = page->myData.myGpuHeapStart.ptr + myHandleIncrementSize * descriptorIndexInHeap;

    DescriptorDX12 descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle = gpuHandle;
    descr.myHeapType = myType;
    descr.myIsManagedByAllocator = true;

    return descr;
  }
//---------------------------------------------------------------------------//
  void StaticDescriptorAllocatorDX12::FreeDescriptor(const DescriptorDX12& aDescriptor)
  {
    ASSERT(aDescriptor.myIsManagedByAllocator && aDescriptor.myHeapType == myType);
    const Page* page = myAllocator.FindPage([this, aDescriptor](const Page& aPage) 
    {
      const uint64 firstHeapAddess = aPage.myData.myCpuHeapStart.ptr;
      const uint64 lastHeapAddress = aPage.myData.myCpuHeapStart.ptr + (aPage.mySize-1) * myHandleIncrementSize;
      return aDescriptor.myCpuHandle.ptr >= firstHeapAddess && aDescriptor.myCpuHandle.ptr <= lastHeapAddress;
    });
    ASSERT(page != nullptr);

    const uint64 addressOffset = aDescriptor.myCpuHandle.ptr - page->myData.myCpuHeapStart.ptr;
    ASSERT(addressOffset % myHandleIncrementSize == 0);

    Block block;
    block.myVirtualOffset = addressOffset / myHandleIncrementSize;
    block.mySize = 1;
    myAllocator.Free(block);
  }
//---------------------------------------------------------------------------//
}
