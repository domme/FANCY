#pragma once

#include "FancyCorePrerequisites.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

namespace Fancy {namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapDX12
  {
    friend class RenderCore_PlatformDX12;

  public:
    explicit DescriptorHeapDX12(const D3D12_DESCRIPTOR_HEAP_DESC& aDesc);

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return myDesc; }
    const uint& GetHandleIncrementSize() const { return myHandleIncrementSize; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHeapStart() const { return myCpuHeapStart; }
    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGpuHeapStart() const { return myGpuHeapStart; }
    ID3D12DescriptorHeap* GetHeap() const { return myDescriptorHeap.Get(); }
    void Reset() { myNextFreeHandleIndex = 0u; }

    DescriptorDX12 AllocateDescriptor();
    DescriptorDX12 GetDescriptor(uint anIndex) const;
    uint GetNumAllocatedDescriptors() const { return myNextFreeHandleIndex; }
    
  private:
    DescriptorHeapDX12();
    void Create(const D3D12_DESCRIPTOR_HEAP_DESC& aDesc);

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC myDesc;

    uint myHandleIncrementSize;
    uint myNextFreeHandleIndex;
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;
  };
//---------------------------------------------------------------------------//
} } }
