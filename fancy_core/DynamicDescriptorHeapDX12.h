#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  /// Shader-visible descriptor heap that can only allocate but not free descriptors. Intended to be used as a temporary staging-heap during commandlist-recording
//---------------------------------------------------------------------------//
  class DynamicDescriptorHeapDX12
  {
    friend class RenderCore_PlatformDX12;

  public:
    struct RangeAllocation
    {
      DynamicDescriptorHeapDX12* myHeap;
      uint myFirstDescriptorIndexInHeap;
      uint myNumDescriptors;
      uint myNumAllocatedDescriptors;
    };

    // Heap layout:
    // [Constant descriptors...|Transient Descriptors]
    // Creates a new descriptor heap of the specified type. A descriptor heap is divided into a constant part (for resource-tables that never change) and a transient part (for dynamic uploads from CPU-only descriptor heaps)
    DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint aNumConstantDescriptors, uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange);

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return myDesc; }
    uint GetHandleIncrementSize() const { return myHandleIncrementSize; }
    ID3D12DescriptorHeap* GetHeap() const { return myDescriptorHeap.Get(); }

    void ResetTransientDescriptors() { myNextFreeTransientDescriptorIdx = myNumConstantDescriptors; }
    uint GetNumFreeTransientDescriptors() const { return (uint) glm::max(0, (int)(myDesc.NumDescriptors - myNextFreeTransientDescriptorIdx)); }

    RangeAllocation AllocateTransientRange();
    void FreeTransientRange(const RangeAllocation& aRange, uint64 aFence);

    DescriptorDX12 AllocateConstantDescriptorRange(uint aNumDescriptors);

    DescriptorDX12 GetDescriptor(uint anIndex) const;

  private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC myDesc;

    uint myNumConstantDescriptors;
    uint myNumTransientDescriptors;
    uint myNumTransientDescriptorsPerRange;

    uint myHandleIncrementSize;
    uint myNextFreeTransientDescriptorIdx;
    uint myNextFreeConstantDescriptorIdx;
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;

    uint myNumTransientRanges;
    eastl::vector<uint64> myTransientRangeLastUseFences;
    std::mutex myRangeAllocMutex;
  };
//---------------------------------------------------------------------------// 
}

#endif