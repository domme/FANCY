#include "fancy_core_precompile.h"
#include "StaticDescriptorAllocatorDX12.h"

#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  namespace Priv_StaticDescirptorAllocatorDX12
  {
    bool locCreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptors, StaticDescriptorAllocatorDX12::Heap& aHeapOut)
    {
      ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();
      D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
      ASSERT(aNumDescriptors <= UINT_MAX);
      heapDesc.NumDescriptors = static_cast<uint>(aNumDescriptors);
      heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      heapDesc.NodeMask = 0u;
      heapDesc.Type = aType;
      
      if (!SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&aHeapOut.myHeap))))
        return false;

      aHeapOut.myCpuHeapStart = aHeapOut.myHeap->GetCPUDescriptorHandleForHeapStart();
      aHeapOut.myGpuHeapStart = aHeapOut.myHeap->GetGPUDescriptorHandleForHeapStart();

      return true;
    }  
  }
//---------------------------------------------------------------------------//
  using Page = PagedLinearAllocator<StaticDescriptorAllocatorDX12::Heap>::Page;
  using Block = PagedLinearAllocator<StaticDescriptorAllocatorDX12::Heap>::Block;
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  StaticDescriptorAllocatorDX12::StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptorsPerHeap)
    : myAllocator(
        aNumDescriptorsPerHeap, 
        [aType](uint64 aNumDescriptors, StaticDescriptorAllocatorDX12::Heap& aHeapOut) { return Priv_StaticDescirptorAllocatorDX12::locCreateDescriptorHeap(aType, aNumDescriptors, aHeapOut); }, 
        [](StaticDescriptorAllocatorDX12::Heap& aHeapToDestroy) { })
    , myHandleIncrementSize(RenderCore::GetPlatformDX12()->GetDevice()->GetDescriptorHandleIncrementSize(aType))
    , myType(aType)
  {
    
  }
//---------------------------------------------------------------------------//
  StaticDescriptorAllocatorDX12::~StaticDescriptorAllocatorDX12()
  {
  #if FANCY_DX12_DEBUG_ALLOCS
    for (auto it : myAllocDebugInfos)
      LOG_WARNING("Leaked static descriptor: % at index %", it.myName.c_str(), it.myVirtualDescriptorIndex);  
#endif

    ASSERT(myAllocator.IsEmpty(), "There are still static descriptors allocated when destroying the descriptor allocator");
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 StaticDescriptorAllocatorDX12::AllocateDescriptor(const char* aDebugName /* = nullptr*/)
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

#if FANCY_DX12_DEBUG_ALLOCS
    AllocDebugInfo debugInfo;
    debugInfo.myName = aDebugName;
    debugInfo.myVirtualDescriptorIndex = static_cast<uint>(descriptorIndexInHeap + page->myStart);
    myAllocDebugInfos.push_back(debugInfo);
#endif

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
    block.myStart = addressOffset / myHandleIncrementSize;
    block.myEnd = 1;
    myAllocator.Free(block);

 #if FANCY_DX12_DEBUG_ALLOCS
    auto it = std::find_if(myAllocDebugInfos.begin(), myAllocDebugInfos.end(), [&block](const AllocDebugInfo& anInfo)
    {
      return anInfo.myVirtualDescriptorIndex == block.myStart;
    });

    ASSERT(it != myAllocDebugInfos.end());
    myAllocDebugInfos.erase(it);
#endif
  }
//---------------------------------------------------------------------------//
}
