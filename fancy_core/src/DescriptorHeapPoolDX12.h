#pragma once

#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "Descriptor.h"
#include <queue>

namespace Fancy { namespace Rendering {
  class RendererDX12;
} }

namespace Fancy {namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  class DescriptorHeapDX12
  {
    friend class DescriptorHeapPoolDX12;

  public:
    DescriptorHeapDX12(ID3D12Device* aDevice, const D3D12_DESCRIPTOR_HEAP_DESC& aDesc);

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return myDesc; }
    const uint& GetHandleIncrementSize() const { return myHandleIncrementSize; }
    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHeapStart() const { return myCpuHeapStart; }
    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGpuHeapStart() const { return myGpuHeapStart; }
    ID3D12DescriptorHeap* GetHeap() const { return myDescriptorHeap.Get(); }
    void Reset() { myNextFreeHandleIndex = 0u; }

    Descriptor AllocateDescriptor();
    Descriptor GetDescriptor(uint32 anIndex);
    uint32 GetNumAllocatedDescriptors() { return myNextFreeHandleIndex; }


  private:
    DescriptorHeapDX12();
    void Create(ID3D12Device* aDevice, const D3D12_DESCRIPTOR_HEAP_DESC& aDesc);

    ComPtr<ID3D12DescriptorHeap> myDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC myDesc;

    uint myHandleIncrementSize;
    uint myNextFreeHandleIndex;
    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;
  };
//---------------------------------------------------------------------------//
  class DescriptorHeapPoolDX12
  {
  public:
    
    static const uint32 kGpuDescriptorNumIncrement = 16u;

    explicit DescriptorHeapPoolDX12(RenderOutputDX12& aRenderer);
    ~DescriptorHeapPoolDX12();

    DescriptorHeapDX12* AllocateDynamicHeap(uint32 aRequiredNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE aHeapType);
    void ReleaseDynamicHeap(uint64 aFenceVal, DescriptorHeapDX12* aUsedHeap);

    DescriptorHeapDX12* GetStaticHeap(D3D12_DESCRIPTOR_HEAP_TYPE aType) { return &myStaticHeaps[aType]; }
    
  private:
    RenderOutputDX12& myRenderer;

    DescriptorHeapDX12 myStaticHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    std::vector<std::unique_ptr<DescriptorHeapDX12>> myDynamicHeapPool;
    std::deque<DescriptorHeapDX12*> myAvailableDynamicHeaps;
    std::queue<std::pair<uint64, DescriptorHeapDX12*>> myUsedDynamicHeaps;
  };
//---------------------------------------------------------------------------//
} } }
