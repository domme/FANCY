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
    DynamicDescriptorHeapDX12(D3D12_DESCRIPTOR_HEAP_TYPE aType, uint aNumDescriptors);

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return myDesc; }
    const uint& GetHandleIncrementSize() const { return myHandleIncrementSize; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHeapStart() const { return myCpuHeapStart; }
    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGpuHeapStart() const { return myGpuHeapStart; }
    ID3D12DescriptorHeap* GetHeap() const { return myDescriptorHeap.Get(); }
    void Reset() { myNextFreeHandleIndex = 0u; }
    uint GetNumFreeDescriptors() const { return (uint) glm::max(0, (int)(myDesc.NumDescriptors - myNextFreeHandleIndex)); }

    DescriptorDX12 AllocateDescriptorRangeGetFirst(uint aNumDescriptors);
    DescriptorDX12 AllocateDescriptor();
    DescriptorDX12 GetDescriptor(uint anIndex) const;
    uint GetNumAllocatedDescriptors() const { return myNextFreeHandleIndex; }
    
  private:
    DynamicDescriptorHeapDX12();
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC myDesc;

    uint myHandleIncrementSize;
    uint myNextFreeHandleIndex;
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;
  };
//---------------------------------------------------------------------------// 
}

#endif