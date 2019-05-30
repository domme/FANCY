#pragma once

#include "FancyCoreDefines.h"
#include "PagedLinearAllocator.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

namespace Fancy 
{
//---------------------------------------------------------------------------//
  class StaticDescriptorAllocatorDX12 : public PagedLinearAllocator
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
    bool CreatePageData(uint64 aSize, Any& aPageData) override;
    void DestroyPageData(Any& aPageData) override;

    uint myHandleIncrementSize;
    D3D12_DESCRIPTOR_HEAP_TYPE myType;
  };
//---------------------------------------------------------------------------//
}



