#pragma once

#include "FancyCoreDefines.h"
#include "DX12Prerequisites.h"
#include "DescriptorDX12.h"

#if FANCY_ENABLE_DX12

namespace Fancy {
//---------------------------------------------------------------------------//
  // Wraps a shader-visible descriptor heap and manages descriptor-allocations.
  // The heap is split into these sections:
  // [      Bindless Descriptors              ||       Temp Descriptors     ]
  // [Textures|RWTextures|Buffers|RWBuffers   ||                            ]
  //---------------------------------------------------------------------------//
  class ShaderVisibleDescriptorHeapDX12
  {
    friend class RenderCore_PlatformDX12;

  public:
    enum BindlessDescriptorType
    {
      BINDLESS_TEXTURE_2D = 0,
      BINDLESS_RW_TEXTURE_2D,
      BINDLESS_BUFFER,
      BINDLESS_RW_BUFFER,
      BINDLESS_SAMPLER,
      BINDLESS_NUM
    };

    struct RangeAllocation
    {
      ShaderVisibleDescriptorHeapDX12* myHeap;
      uint myFirstDescriptorIndexInHeap;
      uint myNumDescriptors;
      uint myNumAllocatedDescriptors;
    };

    ShaderVisibleDescriptorHeapDX12(uint aNumBindlessTextures, uint aNumBindlessRWTextures, uint aNumBindlessBuffers, uint aNumBindlessRWBuffers, 
      uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange);

    ShaderVisibleDescriptorHeapDX12(uint aNumBindlessSamplers, uint aNumTransientDescriptors, uint aNumTransientDescriptorsPerRange);

    const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return myDesc; }
    uint GetHandleIncrementSize() const { return myHandleIncrementSize; }
    ID3D12DescriptorHeap* GetHeap() const { return myDescriptorHeap.Get(); }

    void ResetTransientDescriptors() { myNextFreeTransientDescriptorIdx = myOverallNumBindlessDescriptors; }
    uint GetNumFreeTransientDescriptors() const { return (uint) glm::max(0, (int)(myDesc.NumDescriptors - myNextFreeTransientDescriptorIdx)); }
    uint GetNumTransientDescriptorsPerRange() const { return myNumTransientDescriptorsPerRange; }

    RangeAllocation AllocateTransientRange();
    void FreeTransientRange(const RangeAllocation& aRange, uint64 aFence);

    DescriptorDX12 AllocateDescriptor(BindlessDescriptorType aType);
    void FreeDescriptorAfterFrame(BindlessDescriptorType aType, const DescriptorDX12& aDescriptor);

    D3D12_GPU_DESCRIPTOR_HANDLE GetBindlessHeapStart(BindlessDescriptorType aType) const;

    DescriptorDX12 GetDescriptor(uint anIndex) const;

  private:
    void Init(D3D12_DESCRIPTOR_HEAP_TYPE aType);

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> myDescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_DESC myDesc;

    uint myOverallNumBindlessDescriptors;
    uint myNumBindlessDescriptors[BINDLESS_NUM];
    uint myNumTransientDescriptors;
    uint myNumTransientDescriptorsPerRange;

    uint myHandleIncrementSize;
    uint myNextFreeTransientDescriptorIdx;

    D3D12_CPU_DESCRIPTOR_HANDLE myCpuHeapStart;
    D3D12_GPU_DESCRIPTOR_HANDLE myGpuHeapStart;
    D3D12_CPU_DESCRIPTOR_HANDLE myBindlessDescriptorCpuHeapStart[BINDLESS_NUM];
    D3D12_GPU_DESCRIPTOR_HANDLE myBindlessDescriptorGpuHeapStart[BINDLESS_NUM];

    PagedLinearAllocator myBindlessAllocators[BINDLESS_NUM];

    uint myNumTransientRanges;
    eastl::vector<uint64> myTransientRangeLastUseFences;
    std::mutex myRangeAllocMutex;

    struct DescriptorFreeData
    {
      BindlessDescriptorType myType;
      uint myOffset;
      uint64 myFrame;
    };

    eastl::vector<uint64> my
  };
//---------------------------------------------------------------------------// 
}

#endif