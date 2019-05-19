#pragma once

#include "FancyCoreDefines.h"
#include "PagedLinearAllocator.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"
#include <list>

namespace Fancy 
{
//---------------------------------------------------------------------------//
  class StaticDescriptorAllocatorDX12
  {
  public:
    struct Heap
    {
      D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
      D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;
      Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myHeap;
    };

    StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptorsPerHeap);
    ~StaticDescriptorAllocatorDX12();

    DescriptorDX12 AllocateDescriptor(const char* aDebugName = nullptr);
    void FreeDescriptor(const DescriptorDX12& aDescriptor);

  private:
    PagedLinearAllocator<Heap> myAllocator;
    uint myHandleIncrementSize;
    D3D12_DESCRIPTOR_HEAP_TYPE myType;

#if FANCY_RENDERER_DEBUG_MEMORY_ALLOCS
    struct AllocDebugInfo
    {
      String myName;
      uint myVirtualDescriptorIndex;
    };
    std::list<AllocDebugInfo> myAllocDebugInfos;
#endif
  };
//---------------------------------------------------------------------------//
}



