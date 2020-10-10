#include "fancy_core_precompile.h"
#include "DynamicDescriptorHeapDX12.h"

#include "DescriptorDX12.h"
#include "RenderCore.h"
#include "RenderCore_PlatformDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
  //---------------------------------------------------------------------------//
  DynamicDescriptorHeapDX12::DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint aNumConstantDescriptors, uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange)
    : myNumConstantDescriptors(aNumConstantDescriptors)
    , myNumTransientDescriptors(aNumTransientDescriptors)
    , myNumTransientDescriptorsPerRange(aNumTransientDescriptorsPerRange)
    , myNextFreeTransientDescriptorIdx(aNumConstantDescriptors)
    , myNumTransientRanges(aNumTransientDescriptors / aNumTransientDescriptorsPerRange)
  {
    ASSERT(aNumTransientDescriptorsPerRange <= aNumTransientDescriptors);

    myTransientRangeLastUseFences.resize(myNumTransientRanges, 0ull);

    const uint numDescriptors = myNumConstantDescriptors + myNumTransientDescriptors;
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    heapDesc.NumDescriptors = numDescriptors;
    heapDesc.Type = aType;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0u;

    ID3D12Device* device = RenderCore::GetPlatformDX12()->GetDevice();

    ASSERT_HRESULT(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&myDescriptorHeap)));
    myDesc = heapDesc;

    myHandleIncrementSize = device->GetDescriptorHandleIncrementSize(aType);
    myCpuHeapStart = myDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    myGpuHeapStart = myDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  }
  //---------------------------------------------------------------------------//
  DynamicDescriptorHeapDX12::RangeAllocation DynamicDescriptorHeapDX12::AllocateTransientRange()
  {
    std::lock_guard<std::mutex> lock(myRangeAllocMutex);

    if (GetNumFreeTransientDescriptors() < myNumTransientDescriptorsPerRange)
    {
      // Wrap around to start
      myNextFreeTransientDescriptorIdx = myNumConstantDescriptors;
    }

    const uint rangeIdx = (myNextFreeTransientDescriptorIdx - myNumConstantDescriptors) / myNumTransientDescriptorsPerRange;
    const uint64 rangeLastUseFence = myTransientRangeLastUseFences[rangeIdx];
    ASSERT(RenderCore::IsFenceDone(rangeLastUseFence), 
      "Trying to allocate a dynamic descriptor range that hasn't been fully processed by the GPU yet. Consider increasing the size of the dynamic (shader visible) descriptor heap");

    RangeAllocation alloc;
    alloc.myHeap = this;
    alloc.myFirstDescriptorIndexInHeap = myNextFreeTransientDescriptorIdx;
    alloc.myNumDescriptors = myNumTransientDescriptorsPerRange;
    alloc.myNumAllocatedDescriptors = 0u;

    myNextFreeTransientDescriptorIdx += myNumTransientDescriptorsPerRange;

    return alloc;
  }
//---------------------------------------------------------------------------//
  void DynamicDescriptorHeapDX12::FreeTransientRange(const RangeAllocation& aRange, uint64 aFence)
  {
    ASSERT(aRange.myHeap == this, "Trying to free a RangeAllocation that doesn't belong to this heap");

    const uint rangeIdx = (aRange.myFirstDescriptorIndexInHeap - myNumConstantDescriptors) / myNumTransientDescriptorsPerRange;
    myTransientRangeLastUseFences[rangeIdx] = glm::max(myTransientRangeLastUseFences[rangeIdx], aFence);
  }
//---------------------------------------------------------------------------//
  DescriptorDX12 DynamicDescriptorHeapDX12::GetDescriptor(uint anIndex) const
  {
    ASSERT(anIndex < myDesc.NumDescriptors);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    cpuHandle.ptr = myCpuHeapStart.ptr + anIndex * myHandleIncrementSize;

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    gpuHandle.ptr = myGpuHeapStart.ptr + anIndex * myHandleIncrementSize;

    DescriptorDX12 descr;
    descr.myCpuHandle = cpuHandle;
    descr.myGpuHandle = gpuHandle;
    descr.myHeapType = myDesc.Type;

    return descr;
  }
//---------------------------------------------------------------------------//
}


#endif