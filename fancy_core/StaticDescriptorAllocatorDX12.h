#pragma once

#include "FancyCoreDefines.h"
#include "PagedLinearAllocator.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

#include "EASTL/any.h"

#if FANCY_ENABLE_DX12

namespace Fancy 
{
//---------------------------------------------------------------------------//
  class StaticDescriptorAllocatorDX12 : public PagedLinearAllocator
  {
  public:
    struct Heap
    {
      D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
      Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myHeap;
    };

    StaticDescriptorAllocatorDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint64 aNumDescriptorsPerHeap);
    ~StaticDescriptorAllocatorDX12();

    DescriptorDX12 AllocateDescriptor(const char* aDebugName = nullptr);
    void FreeDescriptor(const DescriptorDX12& aDescriptor);

  private:
    bool CreatePageData(uint64 aSize, eastl::any& aPageData) override;
    void DestroyPageData(eastl::any& aPageData) override;

    uint myHandleIncrementSize;
    D3D12_DESCRIPTOR_HEAP_TYPE myType;
  };
//---------------------------------------------------------------------------//
}

#endif